#pragma once

#include <vulkan/vulkan.hpp>

#include <boost/filesystem.hpp>

#include "common.h"

namespace gh::detail::swapchain {
struct Holder;
}

namespace gh::detail::shader {
struct Holder;
}

namespace gh::detail {

vk::UniqueRenderPass makeRenderPass(vk::Device& device, vk::Format&);

template <vk::DescriptorType type>
vk::UniqueDescriptorPool makeDescriptorPool(vk::Device& device,
                                            uint32_t descriptorSetCount)
{
  vk::DescriptorPoolSize poolSize = {type, descriptorSetCount};
  return device.createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{
      vk::DescriptorPoolCreateFlags{}, descriptorSetCount, 1, &poolSize});
}

template <vk::DescriptorType type = vk::DescriptorType::eUniformBuffer>
auto initializeDescriptorSets(vk::Device& device, vk::DescriptorPool& pool,
                              vk::DescriptorSetLayout& layout, std::size_t count,
                              std::vector<vk::Buffer> uniformBuffers)
{
  auto result = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo{
      pool, static_cast<uint32_t>(count),
      std::vector<vk::DescriptorSetLayout>{count, layout}.data()});

  for (auto& [descriptor, uniformBuffer] : combine_vectors(result, uniformBuffers)) {
    vk::DescriptorBufferInfo info{uniformBuffer, 0, VK_WHOLE_SIZE};
    auto writeDescriptorSet =
        vk::WriteDescriptorSet{descriptor, 0, 0, 1, type, nullptr, &info, nullptr};
    device.updateDescriptorSets({writeDescriptorSet}, {});
  }
  return std::move(result);
}

namespace pipeline {

struct Holder {
  // This is not really in the best place right now, as it gets recreated uselessly on
  // window resize
  vk::UniqueDescriptorSetLayout descriptorSetLayout;
  vk::UniqueDescriptorPool descriptorPool;
  std::vector<vk::DescriptorSet> descriptorSets;

  vk::UniqueRenderPass renderPass;
  vk::UniquePipelineLayout pipelineLayout;
  vk::UniquePipeline pipeline;

  Holder(vk::Device& device, swapchain::Holder& swapchainData, shader::Holder& shaders,
         std::size_t, std::vector<vk::Buffer>, vk::PolygonMode);

  vk::PolygonMode currentRasterization = vk::PolygonMode::eFill;
};

vk::UniquePipelineLayout makeLayout(vk::Device& device,
                                    std::vector<vk::DescriptorSetLayout> layouts);

} // namespace pipeline

vk::UniquePipeline makeGraphicsPipeline(vk::Device& device,
                                        swapchain::Holder& swapchainData,
                                        shader::Holder& shaders,
                                        vk::RenderPass& renderpass,
                                        vk::PipelineLayout& layout, vk::PolygonMode);

} // namespace gh::detail