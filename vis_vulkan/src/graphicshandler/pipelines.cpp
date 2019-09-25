#include "pipelines.h"

#include "common.h"

#include "rendering.h"
#include "shaders.h"
#include "swapchains.h"

namespace gh::detail {

vk::UniqueRenderPass makeRenderPass(vk::Device& device, vk::Format& format)
{
  vk::AttachmentDescription colorAttachment{
      vk::AttachmentDescriptionFlags(), format,
      vk::SampleCountFlagBits::e1,      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eStore,    vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined,
      vk::ImageLayout::ePresentSrcKHR};

  vk::AttachmentReference colorReference{0, vk::ImageLayout::eColorAttachmentOptimal};

  vk::AttachmentDescription depthAttachment{
      vk::AttachmentDescriptionFlags(),
      vk::Format::eD32Sfloat,
      vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eClear,
      vk::AttachmentStoreOp::eDontCare,
      vk::AttachmentLoadOp::eDontCare,
      vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined,
      vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::AttachmentReference depthReference{1,
                                         vk::ImageLayout::eDepthStencilAttachmentOptimal};

  vk::SubpassDescription subpassDescription{vk::SubpassDescriptionFlags(),
                                            vk::PipelineBindPoint::eGraphics};
  subpassDescription.setColorAttachmentCount(1)
      .setPColorAttachments(&colorReference)
      .setPDepthStencilAttachment(&depthReference);

  std::vector<vk::SubpassDependency> subpassDependecies;
  subpassDependecies.emplace_back(
      VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput,
      vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::AccessFlags(),
      vk::AccessFlagBits::eColorAttachmentRead |
          vk::AccessFlagBits::eColorAttachmentWrite);

  std::vector<vk::AttachmentDescription> attachments{colorAttachment, depthAttachment};

  vk::RenderPassCreateInfo createInfo{
      vk::RenderPassCreateFlags(), static_cast<uint32_t>(attachments.size()),
      attachments.data(),          1,
      &subpassDescription,         static_cast<uint32_t>(subpassDependecies.size()),
      subpassDependecies.data()};
  return device.createRenderPassUnique(createInfo);
}

namespace pipeline {

vk::UniqueDescriptorSetLayout makeDescriptorSetLayout(vk::Device& device)
{
  vk::DescriptorSetLayoutBinding uniformBufferBinding{
      0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex};

  std::vector<vk::DescriptorSetLayoutBinding> bindings = {uniformBufferBinding};
  return device.createDescriptorSetLayoutUnique(vk::DescriptorSetLayoutCreateInfo{
      vk::DescriptorSetLayoutCreateFlags(), static_cast<uint32_t>(bindings.size()),
      bindings.data()});
}

Holder::Holder(vk::Device& device, swapchain::Holder& swapchainData,
               shader::Holder& shaders, std::size_t swapchainImageCount,
               std::vector<vk::Buffer> uniformBuffers, vk::PolygonMode polygonMode)
    : descriptorSetLayout(makeDescriptorSetLayout(device)),
      descriptorPool(makeDescriptorPool<vk::DescriptorType::eUniformBuffer>(
          device, static_cast<uint32_t>(swapchainImageCount))),
      // supposedly descriptor sets get deallocated when the pool is destroyed
      // in which case these needn't be unique handles
      descriptorSets(initializeDescriptorSets(device, descriptorPool.get(),
                                              descriptorSetLayout.get(),
                                              swapchainImageCount, uniformBuffers)),
      renderPass(makeRenderPass(device, swapchainData.format)),
      pipelineLayout(makeLayout(device, {descriptorSetLayout.get()})),
      pipeline(makeGraphicsPipeline(device, swapchainData, shaders, renderPass.get(),
                                    pipelineLayout.get(), polygonMode)),
      currentRasterization(polygonMode)
{
}

vk::UniquePipelineLayout makeLayout(vk::Device& device,
                                    std::vector<vk::DescriptorSetLayout> layouts)
{
  // vk::PushConstantRange pushConstants{
  //   vk::ShaderStageFlagBits::eVertex, 0, sizeof(PushConstants)
  // };

  vk::PipelineLayoutCreateInfo createInfo{
      vk::PipelineLayoutCreateFlags{}, static_cast<uint32_t>(layouts.size()),
      layouts.data()
      // TODO: set up push constants if needed
      // ,1, &pushConstants
  };
  return device.createPipelineLayoutUnique(createInfo);
}

} // namespace pipeline

vk::UniquePipeline
makeGraphicsPipeline(vk::Device& device, swapchain::Holder& swapchainData,
                     shader::Holder& shaders, vk::RenderPass& renderPass,
                     vk::PipelineLayout& layout, vk::PolygonMode polygonMode)
{
  // Shaders
  vk::PipelineShaderStageCreateInfo vertexShader{
      vk::PipelineShaderStageCreateFlags{}, vk::ShaderStageFlagBits::eVertex,
      shaders.vertex.get(), "main", nullptr // TODO: look into specializatonInfo
  };
  vk::PipelineShaderStageCreateInfo fragmentShader{
      vk::PipelineShaderStageCreateFlags{}, vk::ShaderStageFlagBits::eFragment,
      shaders.fragment.get(), "main", nullptr // TODO: look into specializatonInfo
  };
  std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{vertexShader,
                                                              fragmentShader};
  // Fixed functions
  auto bindingDescription = getBindingDescription<0, render::Vertex>();
  auto attributeDescription = render::Vertex::getAttributeDescription<0, 0>();
  vk::PipelineVertexInputStateCreateInfo vertexInput{
      vk::PipelineVertexInputStateCreateFlags(), 1, &bindingDescription, 1,
      &attributeDescription};

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
      vk::PipelineInputAssemblyStateCreateFlags(),
      // vk::PrimitiveTopology::eLineListWithAdjacency
      // vk::PrimitiveTopology::eTriangleFan
      vk::PrimitiveTopology::eTriangleList, false};

  // viewport
  vk::Viewport viewport{0.0f,
                        0.0f,
                        static_cast<float>(swapchainData.extent.width),
                        static_cast<float>(swapchainData.extent.height),
                        0.0f,
                        1.0f};
  vk::Rect2D scissor{vk::Offset2D{0, 0}, swapchainData.extent};
  vk::PipelineViewportStateCreateInfo viewportState{
      vk::PipelineViewportStateCreateFlags{}, 1, &viewport, 1, &scissor};

  vk::PipelineRasterizationStateCreateInfo rasterization{
      vk::PipelineRasterizationStateCreateFlags{},
      false,
      false,
      polygonMode,
      vk::CullModeFlagBits::eNone,
      vk::FrontFace::eClockwise,
      false,
      0.0f,
      0.0f,
      0.0f,
      1.0f // linewidth
  };

  vk::PipelineMultisampleStateCreateInfo multisampling{};

  vk::PipelineDepthStencilStateCreateInfo depthStencil{
      vk::PipelineDepthStencilStateCreateFlags(), true, true, vk::CompareOp::eLess};

  vk::PipelineColorBlendAttachmentState colorAttachment{false};
  colorAttachment.setColorWriteMask(~vk::ColorComponentFlags());

  vk::PipelineColorBlendStateCreateInfo colorblending{
      vk::PipelineColorBlendStateCreateFlags{}, false, vk::LogicOp::eCopy, 1,
      &colorAttachment};

  // TODO: customization option
  // vk::DynamicState::eLineWidth;

  vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};

  vk::GraphicsPipelineCreateInfo createInfo{vk::PipelineCreateFlagBits{},
                                            static_cast<uint32_t>(shaderStages.size()),
                                            shaderStages.data(),
                                            &vertexInput,
                                            &inputAssembly,
                                            nullptr /*&tessalation*/,
                                            &viewportState,
                                            &rasterization,
                                            &multisampling,
                                            &depthStencil,
                                            &colorblending,
                                            nullptr /*&dynamicStateInfo*/,
                                            layout,
                                            renderPass,
                                            0,
                                            vk::Pipeline{},
                                            0};

  vk::UniquePipelineCache pipelineCache{};
  return device.createGraphicsPipelineUnique(pipelineCache.get(), createInfo);
}

} // namespace gh::detail