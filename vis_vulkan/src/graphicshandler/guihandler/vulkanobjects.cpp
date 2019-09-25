#include "vulkanobjects.h"

#include <limits>

#include <boost/dll.hpp>

#include "buffers.h"
#include "pipelines.h"
#include "vulkanimpl.h"

#include "common.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace gui::detail {

constexpr auto INITIAL_SIZE = 1024;

vk::UniquePipelineLayout
makeGuiPipelineLayout(vk::Device& device,
                      std::vector<vk::DescriptorSetLayout> descriptorSetLayouts,
                      std::vector<vk::PushConstantRange> pushConstantRanges)
{
  return device.createPipelineLayoutUnique(vk::PipelineLayoutCreateInfo{
      vk::PipelineLayoutCreateFlags{}, static_cast<uint32_t>(descriptorSetLayouts.size()),
      descriptorSetLayouts.data(), static_cast<uint32_t>(pushConstantRanges.size()),
      pushConstantRanges.data()});
}

Buffers::Buffers(vk::PhysicalDevice& physicalDevice, vk::Device& device)
    : physicalDevice(physicalDevice), device(device)
{
  recreateVertexBuffer(INITIAL_SIZE);
  recreateIndexBuffer(INITIAL_SIZE);
}

void Buffers::recreateVertexBuffer(std::size_t newSize)
{
  device.waitIdle();
  mappedVertexMemory = nullptr;
  if (vertexMemory) {
    device.unmapMemory(vertexMemory.get());
  }
  vertexBuffer =
      gh::detail::buffer::createBuffer<vk::BufferUsageFlagBits::eVertexBuffer,
                                       vk::SharingMode::eExclusive, ImDrawVert>(device,
                                                                                newSize);
  vertexMemory = gh::detail::buffer::allocateAndBindMemory(
      physicalDevice, device, vertexBuffer.get(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);
  mappedVertexMemory = reinterpret_cast<ImDrawVert*>(
      device.mapMemory(vertexMemory.get(), 0, VK_WHOLE_SIZE));
  vertexSize = newSize;
}

void Buffers::recreateIndexBuffer(std::size_t newSize)
{
  device.waitIdle();
  mappedIndexMemory = nullptr;
  if (indexMemory) {
    device.unmapMemory(indexMemory.get());
  }
  indexBuffer = gh::detail::buffer::createBuffer<vk::BufferUsageFlagBits::eIndexBuffer,
                                                 vk::SharingMode::eExclusive, uint32_t>(
      device, newSize);
  indexMemory = gh::detail::buffer::allocateAndBindMemory(
      physicalDevice, device, indexBuffer.get(),
      vk::MemoryPropertyFlagBits::eHostVisible |
          vk::MemoryPropertyFlagBits::eHostCoherent);
  mappedIndexMemory =
      reinterpret_cast<uint32_t*>(device.mapMemory(indexMemory.get(), 0, VK_WHOLE_SIZE));
  indexSize = newSize;
}

void Buffers::uploadDrawData(ImDrawData& drawData)
{
  if (vertexSize < drawData.TotalVtxCount) {
    recreateVertexBuffer(drawData.TotalVtxCount);
  }
  if (indexSize < drawData.TotalIdxCount) {
    recreateIndexBuffer(drawData.TotalIdxCount);
  }

  auto vertexMap = mappedVertexMemory;
  auto indexMap = mappedIndexMemory;
  for (auto i = 0; i < drawData.CmdListsCount; ++i) {
    auto commandList = drawData.CmdLists[i];
    for (auto& vtx : commandList->VtxBuffer) {
      *vertexMap = vtx;
      ++vertexMap;
    }
    for (auto& idx : commandList->IdxBuffer) {
      *indexMap = static_cast<uint32_t>(idx);
      ++indexMap;
    }
  }
}

void VulkanObjects::recordEmptyCommandBuffer(std::size_t i)
{
  vk::CommandBufferInheritanceInfo emptyInheritanceInfo{renderPass};
  guiBuffers[i]->begin(vk::CommandBufferBeginInfo{
      vk::CommandBufferUsageFlagBits::eRenderPassContinue, &emptyInheritanceInfo});
  guiBuffers[i]->end();
}

void VulkanObjects::recordGuiSecondaryBuffer(std::size_t index, ImDrawData& drawData)
{
  auto buffer = guiBuffers[index].get();
  auto framebuffer = vulkanImpl.framebuffers[index].get();
  vk::CommandBufferInheritanceInfo inheritanceInfo{renderPass, 0, framebuffer};
  buffer.begin(vk::CommandBufferBeginInfo{
      vk::CommandBufferUsageFlagBits::eRenderPassContinue, &inheritanceInfo});
  {
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, guiPipeline.get());

    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout.get(), 0,
                              {descriptorSets.front()}, {});
    buffer.bindVertexBuffers(0, {buffers.vertexBuffer.get()}, {0u});
    buffer.bindIndexBuffer(buffers.indexBuffer.get(), 0, vk::IndexType::eUint32);

    buffer.setViewport(0, {vk::Viewport{0.0f, 0.0f, drawData.DisplaySize.x,
                                        drawData.DisplaySize.y, 0.0f, 1.0f}});

    glm::vec2 scale{2.0f / drawData.DisplaySize.x, 2.0f / drawData.DisplaySize.y};
    glm::vec2 translate{-1.0f - drawData.DisplayPos.x * scale.x,
                        -1.0f - drawData.DisplayPos.y * scale.y};
    buffer.pushConstants<PushConstants>(pipelineLayout.get(),
                                        vk::ShaderStageFlagBits::eVertex, 0,
                                        {PushConstants{scale, translate}});

    auto vertexOffset = 0;
    auto indexOffset = 0;
    for (auto listIndex = 0; listIndex < drawData.CmdListsCount; ++listIndex) {
      auto commandList = drawData.CmdLists[listIndex];
      if (commandList == nullptr) {
        continue;
      }
      for (auto bufferIndex = 0; bufferIndex < commandList->CmdBuffer.Size;
           ++bufferIndex) {
        auto drawCommand = commandList->CmdBuffer[bufferIndex];
        auto displayPos = drawData.DisplayPos;
        vk::Rect2D scissor{
            vk::Offset2D{
                clamp(0, std::numeric_limits<int32_t>::max(),
                      static_cast<int32_t>(drawCommand.ClipRect.x - displayPos.x)),
                clamp(0, std::numeric_limits<int32_t>::max(),
                      static_cast<int32_t>(drawCommand.ClipRect.y - displayPos.y))},
            vk::Extent2D{
                static_cast<uint32_t>(drawCommand.ClipRect.z - drawCommand.ClipRect.x),
                static_cast<uint32_t>(drawCommand.ClipRect.w - drawCommand.ClipRect.y)}};
        buffer.setScissor(0, {scissor});

        buffer.drawIndexed(drawCommand.ElemCount, 1, indexOffset, vertexOffset, 0);
        indexOffset += drawCommand.ElemCount;
      }
      vertexOffset += commandList->VtxBuffer.Size;
    }
  }
  buffer.end();
}

void FontObjects::createImage(std::size_t width, std::size_t height)
{
  image = device.createImageUnique(vk::ImageCreateInfo{
      vk::ImageCreateFlags{}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Unorm,
      vk::Extent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1u}, 1u,
      1u, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal,
      vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
      vk::SharingMode::eExclusive});
  auto memoryRequirements = device.getImageMemoryRequirements(image.get());
  imageMemory = device.allocateMemoryUnique(vk::MemoryAllocateInfo{
      memoryRequirements.size,
      gh::detail::findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits,
                                 vk::MemoryPropertyFlagBits::eDeviceLocal)});
  device.bindImageMemory(image.get(), imageMemory.get(), 0);
}

void FontObjects::createImageView()
{
  vk::ImageViewCreateInfo createInfo{
      vk::ImageViewCreateFlags{},
      image.get(),
      vk::ImageViewType::e2D,
      vk::Format::eR8G8B8A8Unorm,
      vk::ComponentMapping{},
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};

  imageView = device.createImageViewUnique(createInfo);
}

void VulkanObjects::setupFonts()
{
  auto& io = ImGui::GetIO();
  // sampler is created in constructor
  unsigned char* pixels;
  int width, height;
  io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
  std::size_t bitCount = width * height * 4;

  fontObjects.createImage(width, height);
  fontObjects.createImageView();
  // update descriptor set
  vk::DescriptorImageInfo imageInfo{fontObjects.sampler.get(),
                                    fontObjects.imageView.get(),
                                    vk::ImageLayout::eShaderReadOnlyOptimal};
  vk::WriteDescriptorSet writeDescriptorSet{descriptorSets.front(),
                                            0u,
                                            0u,
                                            1,
                                            vk::DescriptorType::eCombinedImageSampler,
                                            &imageInfo};
  device.updateDescriptorSets({writeDescriptorSet}, {});

  // upload image
  uploadBuffer = gh::detail::buffer::createBuffer<vk::BufferUsageFlagBits::eTransferSrc,
                                                  vk::SharingMode::eExclusive, char>(
      device, bitCount);
  uploadBufferMemory = gh::detail::buffer::allocateAndBindMemory(
      physicalDevice, device, {uploadBuffer.get()},
      vk::MemoryPropertyFlagBits::eHostVisible);
  // fill buffer
  auto data =
      static_cast<char*>(device.mapMemory(uploadBufferMemory.get(), 0, VK_WHOLE_SIZE));
  {
    memcpy(data, pixels, bitCount);
    device.flushMappedMemoryRanges(
        {vk::MappedMemoryRange{uploadBufferMemory.get(), 0, bitCount}});
  }
  device.unmapMemory(uploadBufferMemory.get());

  // allocate and record command buffer for upload
  auto oneElementVector =
      device.allocateCommandBuffersUnique(vk::CommandBufferAllocateInfo{
          vulkanImpl.commandHolder.pool.get(), vk::CommandBufferLevel::ePrimary, 1});
  uploadCommandBuffer = std::move(oneElementVector.front());

  uploadCommandBuffer->begin(
      vk::CommandBufferBeginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
  {

    vk::ImageSubresourceRange subresourceRange{vk::ImageAspectFlagBits::eColor, 0u, 1u,
                                               0u, 1u};
    // set up copy barrier
    vk::ImageMemoryBarrier copy{
        vk::AccessFlags{},           vk::AccessFlagBits::eTransferWrite,
        vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal,
        VK_QUEUE_FAMILY_IGNORED,     VK_QUEUE_FAMILY_IGNORED,
        fontObjects.image.get(),     subresourceRange};
    uploadCommandBuffer->pipelineBarrier(vk::PipelineStageFlagBits::eHost,
                                         vk::PipelineStageFlagBits::eTransfer,
                                         vk::DependencyFlags{}, {}, {}, {copy});
    // copy to image from buffer
    vk::BufferImageCopy region{};
    region.imageSubresource =
        vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0u, 0u, 1u};
    region.imageExtent =
        vk::Extent3D{static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1};
    uploadCommandBuffer->copyBufferToImage(uploadBuffer.get(), fontObjects.image.get(),
                                           vk::ImageLayout::eTransferDstOptimal,
                                           {region});
    // set up use barrier
    vk::ImageMemoryBarrier use{vk::AccessFlagBits::eTransferWrite,
                               vk::AccessFlagBits::eShaderRead,
                               vk::ImageLayout::eTransferDstOptimal,
                               vk::ImageLayout::eShaderReadOnlyOptimal,
                               VK_QUEUE_FAMILY_IGNORED,
                               VK_QUEUE_FAMILY_IGNORED,
                               fontObjects.image.get(),
                               subresourceRange};
  }
  uploadCommandBuffer->end();
  // submit commandBuffer

  vk::SubmitInfo submitInfo{};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &uploadCommandBuffer.get();

  vulkanImpl.graphicsQueue.submit({submitInfo}, vk::Fence{});

  // tell ImGui about the image
  io.Fonts->TexID = static_cast<ImTextureID>(fontObjects.image.get());
}

VulkanObjects::VulkanObjects(gh::detail::HandlerImpl& vulkanImpl)
    : physicalDevice(vulkanImpl.physicalDevice), device(vulkanImpl.device.get()),
      guiBuffers(gh::detail::command::makeBuffers<vk::CommandBufferLevel::eSecondary>(
          device, vulkanImpl.commandHolder.pool.get(), vulkanImpl.framebuffers.size())),
      renderPass(vulkanImpl.pipelineHolder.renderPass.get()),
      descriptorPool(
          gh::detail::makeDescriptorPool<vk::DescriptorType::eCombinedImageSampler>(
              device, vulkanImpl.images.swapchainImages.size())),
      buffers(physicalDevice, device), vulkanImpl(vulkanImpl),
      fontObjects(physicalDevice, device)
{
  auto device = vulkanImpl.device.get();
  auto guiShaderDirectory = std::filesystem::current_path() / "share" / EXECUTABLE_NAME / "gui_shaders";
  shaders.emplace(vk::ShaderStageFlagBits::eVertex,
                  gh::detail::makeShaderModule(device, guiShaderDirectory / "vert.spv"));

  shaders.emplace(vk::ShaderStageFlagBits::eFragment,
                  gh::detail::makeShaderModule(device, guiShaderDirectory / "frag.spv"));

  fontObjects.sampler = device.createSamplerUnique(vk::SamplerCreateInfo{
      vk::SamplerCreateFlags{}, vk::Filter::eLinear, vk::Filter::eLinear,
      vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat,
      vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat});

  vk::DescriptorSetLayoutBinding binding{0u, vk::DescriptorType::eCombinedImageSampler,
                                         1u, vk::ShaderStageFlagBits::eFragment,
                                         &fontObjects.sampler.get()};

  descriptorSetLayout =
      device.createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo{
          vk::DescriptorSetLayoutCreateFlags{}, 1, &binding});

  descriptorSets = device.allocateDescriptorSets(
      vk::DescriptorSetAllocateInfo{descriptorPool.get(), 1, &descriptorSetLayout.get()});

  pipelineLayout =
      makeGuiPipelineLayout(device, {descriptorSetLayout.get()},
                            {vk::PushConstantRange{vk::ShaderStageFlagBits::eVertex, 0,
                                                   sizeof(PushConstants)}});
  // create gui pipeline
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
  for (auto& [stage, module] : shaders) {
    shaderStages.emplace_back(vk::PipelineShaderStageCreateFlags{}, stage, module.get(),
                              "main");
  }

  vk::VertexInputBindingDescription bindingDescription{0u, sizeof(ImDrawVert)};
  std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
  attributeDescriptions.emplace_back(0u, bindingDescription.binding,
                                     vk::Format::eR32G32Sfloat,
                                     IM_OFFSETOF(ImDrawVert, pos));
  attributeDescriptions.emplace_back(1u, bindingDescription.binding,
                                     vk::Format::eR32G32Sfloat,
                                     IM_OFFSETOF(ImDrawVert, uv));
  attributeDescriptions.emplace_back(2u, bindingDescription.binding,
                                     vk::Format::eR8G8B8A8Unorm,
                                     IM_OFFSETOF(ImDrawVert, col));
  vk::PipelineVertexInputStateCreateInfo vertexInfo{
      vk::PipelineVertexInputStateCreateFlags{}, 1u, &bindingDescription,
      static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data()};

  vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
      vk::PipelineInputAssemblyStateCreateFlags{}, vk::PrimitiveTopology::eTriangleList};

  vk::PipelineViewportStateCreateInfo viewportInfo{
      vk::PipelineViewportStateCreateFlags{}, 1u, nullptr, 1u,
      nullptr // viewports and scissors will be dynamic states
  };

  vk::PipelineRasterizationStateCreateInfo rasterInfo{
      vk::PipelineRasterizationStateCreateFlags{},
      false,
      false,
      vk::PolygonMode::eFill,
      vk::CullModeFlagBits::eNone,
      vk::FrontFace::eCounterClockwise};
  rasterInfo.lineWidth = 1.0f;

  vk::PipelineMultisampleStateCreateInfo multisampleInfo{};

  vk::PipelineColorBlendAttachmentState colorAttachment{
      true,
      vk::BlendFactor::eSrcAlpha,
      vk::BlendFactor::eOneMinusSrcAlpha,
      vk::BlendOp::eAdd,
      vk::BlendFactor::eOneMinusSrcAlpha,
      vk::BlendFactor::eZero,
      vk::BlendOp::eAdd,
      ~vk::ColorComponentFlags{}};

  vk::PipelineDepthStencilStateCreateInfo depthInfo{};

  vk::PipelineColorBlendStateCreateInfo blendInfo{};
  blendInfo.attachmentCount = 1u;
  blendInfo.pAttachments = &colorAttachment;

  std::vector<vk::DynamicState> dynamicStates{vk::DynamicState::eViewport,
                                              vk::DynamicState::eScissor};
  vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
      vk::PipelineDynamicStateCreateFlags{}, static_cast<uint32_t>(dynamicStates.size()),
      dynamicStates.data()};

  vk::PipelineCache pipelineCache{};

  guiPipeline = device.createGraphicsPipelineUnique(
      pipelineCache,
      vk::GraphicsPipelineCreateInfo{
          vk::PipelineCreateFlags{}, static_cast<uint32_t>(shaderStages.size()),
          shaderStages.data(), &vertexInfo, &inputAssemblyInfo, nullptr, &viewportInfo,
          &rasterInfo, &multisampleInfo, &depthInfo, &blendInfo, &dynamicStateInfo,
          pipelineLayout.get(), renderPass});
}

} // namespace gui::detail