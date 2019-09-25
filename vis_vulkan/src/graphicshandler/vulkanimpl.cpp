#include "vulkanimpl.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <optional>
#include <random>
#include <thread>

#include <boost/dll/runtime_symbol_info.hpp>
#include <boost/filesystem.hpp>

#include "common.h"

#include "guihandler.h"
#include "windowhandler.h"

const std::vector<const char*> deviceExtensions{
    // extend as needed
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
    //
};

namespace gh::detail {

template <bool PresentSupportNeeded = false>
std::optional<uint32_t>
getQueueFamilyIndex(vk::PhysicalDevice device, vk::QueueFlagBits bit,
                    std::optional<vk::SurfaceKHR> surface = std::nullopt)
{
  auto queueFamilyProperties = device.getQueueFamilyProperties();
  auto queueFamily =
      std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
                   [bit](auto property) { return property.queueFlags & bit; });
  if (queueFamily == queueFamilyProperties.end()) {
    return std::nullopt;
  }
  auto index =
      static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), queueFamily));
  if (PresentSupportNeeded) {
    if (!surface.has_value()) {
      std::cerr << __FUNCTION__
                << ": Present support requested but no surface passed as argument"
                << std::endl;
      return std::nullopt;
    }
    if (!device.getSurfaceSupportKHR(index, surface.value())) {
      return std::nullopt;
    }
  }
  return std::make_optional(index);
}

auto pickPhysicalDevice(vk::Instance& instance, vk::SurfaceKHR& surface,
                        std::vector<const char*> deviceExtensions)
{
  auto devices = instance.enumeratePhysicalDevices();
  auto device = std::find_if(
      devices.begin(), devices.end(),
      [&deviceExtensions, &surface](vk::PhysicalDevice& d) {
        auto properties = d.getProperties();
        auto features = d.getFeatures();
        auto extensions = d.enumerateDeviceExtensionProperties();

        bool hasGeometryShader = features.geometryShader;
        bool hasNonSolidFillModes = features.fillModeNonSolid;
        bool hasWideLines = features.wideLines;
        bool hasExtensions = std::all_of(
            deviceExtensions.begin(), deviceExtensions.end(), [extensions](auto& ext) {
              for (auto& e : extensions) {
                if (std::string(e.extensionName) == std::string(ext)) {
                  return true;
                }
              }
              return false;
            });
        auto graphicsQueueFamily =
            getQueueFamilyIndex<true>(d, vk::QueueFlagBits::eGraphics, surface);
        if (!graphicsQueueFamily.has_value()) {
          return false;
        }

        return hasGeometryShader && hasExtensions && hasNonSolidFillModes &&
               hasWideLines &&
               swapchain::supported(d, surface, {graphicsQueueFamily.value()});
      });
  if (device == devices.end()) {
    throw std::runtime_error("No suitable physical device found");
  }
  return *device;
}

template <vk::QueueFlagBits QueueFlag = vk::QueueFlagBits::eGraphics>
auto makeDevice(vk::PhysicalDevice& physicalDevice)
{
  vk::PhysicalDeviceFeatures features{};
  features.setWideLines(true).setFillModeNonSolid(true);

  float priority = 1.0f;
  vk::DeviceQueueCreateInfo queueCreateInfo{
      vk::DeviceQueueCreateFlags(),
      getQueueFamilyIndex(physicalDevice, QueueFlag).value(), 1, &priority};
  vk::DeviceCreateInfo deviceCreateInfo{vk::DeviceCreateFlags(),
                                        1,
                                        &queueCreateInfo,
                                        0,
                                        nullptr,
                                        static_cast<uint32_t>(deviceExtensions.size()),
                                        deviceExtensions.data(),
                                        &features};

  return physicalDevice.createDeviceUnique(deviceCreateInfo);
}

auto createFrameBuffers(vk::Device& device, std::vector<vk::ImageView> views,
                        vk::ImageView& depthView, vk::Extent2D& extent,
                        vk::RenderPass& renderPass)
{
  std::vector<vk::UniqueFramebuffer> result;
  result.reserve(views.size());
  for (auto& view : views) {
    std::vector<vk::ImageView> attachments{view, depthView};
    vk::FramebufferCreateInfo createInfo{
        vk::FramebufferCreateFlags{},
        renderPass,
        static_cast<uint32_t>(attachments.size()),
        attachments.data(),
        extent.width,
        extent.height,
        1 // layers
    };
    result.emplace_back(device.createFramebufferUnique(createInfo));
  }
  return result;
}

template <typename Container>
auto processShaderDirectoryContents(vk::Device& device, const Container& shaderDirs)
{
  static_assert(std::is_same_v<typename Container::value_type, ShaderDirectoryContents>);
  std::vector<shader::Holder> result;
  for (auto& contents : shaderDirs) {
    result.emplace_back(makeShaderModule(device, contents.at(FileType::vertexShader)),
                        makeShaderModule(device, contents.at(FileType::fragmentShader)),
                        readIndices(contents.at(FileType::indicesList)),
                        contents.at(FileType::directory).string());
  }
  return result;
}

HandlerImpl::HandlerImpl(const std::string& appName, WindowHandler& windowHandler,
                         std::vector<ShaderDirectoryContents>& shaderDirs,
                         std::vector<const char*> extensions,
                         std::vector<const char*> layers, std::size_t vertexCount,
                         std::size_t history)
    : appInfo(appName.c_str(), APP_VERSION), historySize(history),
      vertices(vertexCount * history, std::complex<float>(0, 0)),
      instance(vk::createInstanceUnique(
          vk::InstanceCreateInfo(vk::InstanceCreateFlags(), &appInfo, layers.size(),
                                 layers.data(), extensions.size(), extensions.data()))),
      debugInfo(instance.get()),
      surface(std::any_cast<VkSurfaceKHR>(
          windowHandler.createSurface(static_cast<VkInstance>(instance.get())))),
      physicalDevice(pickPhysicalDevice(instance.get(), surface, deviceExtensions)),
      device(makeDevice(physicalDevice)),
      graphicsQueue(device->getQueue(
          getQueueFamilyIndex<true>(physicalDevice, vk::QueueFlagBits::eGraphics, surface)
              .value(),
          0)),
      swapchainHolder(physicalDevice, device.get(), surface),
      images(physicalDevice, device.get(), swapchainHolder),
      shaders(processShaderDirectoryContents(device.get(), shaderDirs)),
      currentShaderIt(shaders.begin()),
      bufferHolder(physicalDevice, device.get(), vertices.size(),
                   std::max_element(shaders.begin(), shaders.end(),
                                    [](auto& shader1, auto& shader2) {
                                      return shader1.indexList.size() <
                                             shader2.indexList.size();
                                    })
                       ->indexList.size(),
                   images.swapchainImages.size()),
      pipelineHolder(device.get(), swapchainHolder, *currentShaderIt,
                     images.swapchainImages.size(),
                     getHandles<vk::Buffer>(bufferHolder.uniform), polygonMode),
      framebuffers(createFrameBuffers(
          device.get(), getHandles<vk::ImageView>(images.swapchainImageViews),
          images.depth.view.get(), swapchainHolder.extent,
          pipelineHolder.renderPass.get())),
      commandHolder(
          device.get(),
          vk::CommandPoolCreateFlagBits::eResetCommandBuffer |
              vk::CommandPoolCreateFlagBits::eTransient,
          getQueueFamilyIndex(physicalDevice, vk::QueueFlagBits::eGraphics).value(),
          pipelineHolder, framebuffers.size(), swapchainHolder.extent),
      sync(device.get(), images.swapchainImages.size()), windowHandler(windowHandler)
{
  std::clog << __FUNCTION__ << std::endl;
  auto indices = currentShaderIt->indexList;
  render::UniformBufferObject::vertexCount = vertices.size();
  render::UniformBufferObject::rowCount = historySize;
  // record command buffers
  auto framebufferHandles = getHandles<vk::Framebuffer>(framebuffers);
  command::recordSceneSecondaryBuffers(commandHolder.sceneBuffers, pipelineHolder,
                                       bufferHolder, framebufferHandles);
  updateIndices();
  mappedVertices = reinterpret_cast<render::Vertex*>(
      device->mapMemory(bufferHolder.memory.get(), 0, VK_WHOLE_SIZE));
  updateVertices();
}

void HandlerImpl::updateIndices()
{
  auto& indices = currentShaderIt->indexList;
  auto indexPtr = reinterpret_cast<uint32_t*>(
      device->mapMemory(bufferHolder.indexMemory.get(), 0, VK_WHOLE_SIZE));
  std::for_each(indices.begin(), indices.end(),
                [&indexPtr](auto& index) { *indexPtr++ = index; });
  bufferHolder.indexCount = indices.size();
  device->unmapMemory(bufferHolder.indexMemory.get());
}

void HandlerImpl::updateVertices()
{
  auto vertexPtr = mappedVertices;
  // for (auto& vertex : vertices) {
  //   *vertexPtr++ = vertex;
  // }
  std::for_each(vertices.begin(), vertices.end(),
                [&vertexPtr](auto& vertex) { *vertexPtr++ = vertex; });
}

constexpr auto uint64max = static_cast<uint64_t>(std::numeric_limits<uint64_t>::max());

void HandlerImpl::onOutOfDateSwapchain(int width, int height)
{
  // std::clog << __FUNCTION__ << ": " << std::endl;

  swapchainHolder.recreateSwapchain(width, height);
  auto& newSwapchain = swapchainHolder;

  auto newImageViews = images.recreateSwapchainImages(newSwapchain);
  auto newDepth =
      image::makeDepthImage(physicalDevice, device.get(), newSwapchain.extent);

  pipeline::Holder newPipeline{device.get(),
                               newSwapchain,
                               *currentShaderIt,
                               images.swapchainImages.size(),
                               getHandles<vk::Buffer>(bufferHolder.uniform),
                               polygonMode};

  auto newFramebuffers = createFrameBuffers(
      device.get(), getHandles<vk::ImageView>(newImageViews), newDepth.view.get(),
      newSwapchain.extent, newPipeline.renderPass.get());

  auto newCommandBuffers = command::makeBuffers<vk::CommandBufferLevel::ePrimary>(
      device.get(), commandHolder.pool.get(), newFramebuffers.size());
  auto newSecondaryCommandBuffers =
      command::makeBuffers<vk::CommandBufferLevel::eSecondary>(
          device.get(), commandHolder.pool.get(), newFramebuffers.size());
  auto framebufferHandles = getHandles<vk::Framebuffer>(newFramebuffers);
  command::recordSceneSecondaryBuffers(newSecondaryCommandBuffers, newPipeline,
                                       bufferHolder, framebufferHandles);

  command::recordPrimaryBuffers<vk::CommandBufferUsageFlagBits::eSimultaneousUse>(
      newCommandBuffers, newPipeline, framebufferHandles, newSwapchain.extent,
      getHandles<vk::CommandBuffer>(newSecondaryCommandBuffers));

  // TODO: is there a way to not wait
  graphicsQueue.waitIdle();

  commandHolder.buffers = std::move(newCommandBuffers);
  commandHolder.sceneBuffers = std::move(newSecondaryCommandBuffers);
  framebuffers = std::move(newFramebuffers);
  pipelineHolder = std::move(newPipeline);
  images.swapchainImageViews = std::move(newImageViews);
  images.depth = std::move(newDepth);
}

HandlerImpl::~HandlerImpl()
{
  device->waitIdle();
  // unmap vertices
  device->unmapMemory(bufferHolder.memory.get());
}

void HandlerImpl::drawFrame()
{
  static std::size_t currentFrame = 0;
  // std::clog << __FUNCTION__ << ": Current frame index: " << currentFrame << std::endl;
  if (guiHandler != nullptr) {
    guiHandler->startFrame();
  }
  // waiting for resources
  auto currentFence = sync.inFlight[currentFrame].get();
  device->waitForFences({currentFence}, true, uint64max);
  device->resetFences({currentFence});

  // rendering
  auto currentImageReadySemaphore = sync.imageReady[currentFrame].get();
  auto currentRenderFinishedSemaphore = sync.renderFinished[currentFrame].get();

  uint32_t imageIndex;
  // There is an other overload for this, but that throws if the window was resized...
  auto result =
      device->acquireNextImageKHR(swapchainHolder.swapchain.get(), uint64max,
                                  currentImageReadySemaphore, vk::Fence(), &imageIndex);
  // std::clog << "vk::Device::acquireNextImageKHR result: " << vk::to_string(result)
  //           << std::endl;
  switch (result) {
  case vk::Result::eSuccess:
    break;
  case vk::Result::eErrorOutOfDateKHR: {
    std::clog << vk::to_string(result) << ": Recreating swapchain" << std::endl;
    auto [newWidth, newHeight] = windowHandler.getFrameBufferSize();
    onOutOfDateSwapchain(newWidth, newHeight);
    break;
  }
  default:
    break;
  }

  guiHandler->render(imageIndex);

  command::recordPrimaryBuffer<vk::CommandBufferUsageFlagBits::eOneTimeSubmit>(
      commandHolder.buffers[imageIndex].get(),
      {commandHolder.sceneBuffers[imageIndex].get(),
       guiHandler->getCommandBuffers()[imageIndex]},
      swapchainHolder.extent, framebuffers[imageIndex].get(), pipelineHolder);

  auto uniformBufferData =
      getUniformBufferData(swapchainHolder.extent.width, swapchainHolder.extent.height);
  bufferHolder.writeUniformBufferData(imageIndex, std::move(uniformBufferData));

  std::vector<vk::Semaphore> waitSemaphores{currentImageReadySemaphore};
  auto pipelineStage = static_cast<vk::PipelineStageFlags>(
      vk::PipelineStageFlagBits::eColorAttachmentOutput);
  std::vector<vk::Semaphore> signalSemaphores{currentRenderFinishedSemaphore};
  std::vector<vk::SubmitInfo> submitInfo;
  submitInfo.emplace_back(waitSemaphores.size(), waitSemaphores.data(), &pipelineStage, 1,
                          &commandHolder.buffers[imageIndex].get(),
                          signalSemaphores.size(), signalSemaphores.data());
  if (result != vk::Result::eErrorOutOfDateKHR) {
    graphicsQueue.submit(submitInfo, currentFence);
  }
  else {
    sync.inFlight[currentFrame].reset(
        device->createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled)));
  }
  // presenting
  std::vector<vk::SwapchainKHR> swapchains{swapchainHolder.swapchain.get()};
  vk::PresentInfoKHR presentInfo{
      static_cast<uint32_t>(signalSemaphores.size()), signalSemaphores.data(),
      static_cast<uint32_t>(swapchains.size()), swapchains.data(), &imageIndex};
  if (result != vk::Result::eErrorOutOfDateKHR) {
    graphicsQueue.presentKHR(presentInfo);
  }

  (++currentFrame) %= images.swapchainImages.size();
}

void HandlerImpl::setGuiHandler(GuiHandler& guiHandler)
{
  this->guiHandler = &guiHandler;
  this->guiHandler->onRasterizationChange([this](auto raster) {
    std::clog << "new raster: " << vk::to_string(raster) << std::endl;
    graphicsQueue.waitIdle();
    pipelineHolder.currentRasterization = raster;
    pipelineHolder.pipeline = makeGraphicsPipeline(
        device.get(), swapchainHolder, *currentShaderIt, pipelineHolder.renderPass.get(),
        pipelineHolder.pipelineLayout.get(), raster);

    command::recordSceneSecondaryBuffers(commandHolder.sceneBuffers, pipelineHolder,
                                         bufferHolder,
                                         getHandles<vk::Framebuffer>(framebuffers));
  });
  this->guiHandler->onScaleChange([this](auto scale) {
    std::clog << "new scale: " << to_string(scale) << std::endl;
    switch (scale) {
    case ScaleType::linear:
      scaleFunction = scale::linear<std::complex<float>>;
      break;
    case ScaleType::exponential:
      scaleFunction = scale::exponential<std::complex<float>>;
      break;
    case ScaleType::logarithmic:
      scaleFunction = scale::logarithmic<std::complex<float>>;
      break;
    }
  });
  this->guiHandler->onShaderChange([this](auto name) {
    std::clog << "new shader: " << name << std::endl;
    auto it = std::find_if(shaders.begin(), shaders.end(), [&name](auto& shaderHolder) {
      return shaderHolder.name == name;
    });
    if (it == shaders.end()) {
      std::cerr << "This should not happen" << std::endl;
      return;
    }
    graphicsQueue.waitIdle();
    currentShaderIt = it;
    updateIndices();
    pipelineHolder.pipeline = makeGraphicsPipeline(
        device.get(), swapchainHolder, *currentShaderIt, pipelineHolder.renderPass.get(),
        pipelineHolder.pipelineLayout.get(), pipelineHolder.currentRasterization);
    command::recordSceneSecondaryBuffers(commandHolder.sceneBuffers, pipelineHolder,
                                         bufferHolder,
                                         getHandles<vk::Framebuffer>(framebuffers));
  });
}

void HandlerImpl::addData(std::vector<std::complex<float>> data)
{
  // std::clog << __FUNCTION__ << ": data.size(" << data.size() << ")" << std::endl;
  std::size_t index = 0;
  std::size_t sampleCount = data.size();
  for (auto it = ++std::move_iterator(data.begin()); it != std::move_iterator(data.end());
       ++it, ++index) {
    auto x = static_cast<float>(index) / sampleCount;
    vertices.push_back(scaleFunction(*it));
  }
  updateVertices();
}

} // namespace gh::detail