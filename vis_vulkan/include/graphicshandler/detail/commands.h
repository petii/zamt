#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

#include "buffers.h"
#include "pipelines.h"

namespace gh::detail::command {

void recordSceneSecondaryBuffer(vk::CommandBuffer& buffer,
                                pipeline::Holder& pipelineHolder,
                                vk::DescriptorSet& descriptorSet, buffer::Holder& buffers,
                                vk::Framebuffer& framebuffer);

void recordSceneSecondaryBuffers(std::vector<vk::UniqueCommandBuffer>& buffers,
                                 pipeline::Holder& pipelineHolder,
                                 buffer::Holder& bufferHolder,
                                 std::vector<vk::Framebuffer> framebuffers);

template <vk::CommandBufferUsageFlagBits BufferUsage>
void recordPrimaryBuffer(vk::CommandBuffer& buffer,
                         std::vector<vk::CommandBuffer> secondaryBuffers,
                         vk::Extent2D& extent, vk::Framebuffer& framebuffer,
                         pipeline::Holder& pipelineHolder)
{
  buffer.begin(vk::CommandBufferBeginInfo{BufferUsage});
  {
    vk::Rect2D renderArea{vk::Offset2D{0, 0}, extent};
    std::vector<vk::ClearValue> clearValues{vk::ClearValue{},
                                            vk::ClearDepthStencilValue{1.0f, 0}};
    vk::RenderPassBeginInfo renderPassBeginInfo{
        pipelineHolder.renderPass.get(), framebuffer, renderArea,
        static_cast<uint32_t>(clearValues.size()), clearValues.data()};
    buffer.beginRenderPass(renderPassBeginInfo,
                           vk::SubpassContents::eSecondaryCommandBuffers);
    {
      buffer.executeCommands(secondaryBuffers);
    }
    buffer.endRenderPass();
  }
  buffer.end();
}

template <vk::CommandBufferUsageFlagBits BufferUsage, typename... SecondaryBuffers>
void recordPrimaryBuffers(std::vector<vk::UniqueCommandBuffer>& commandBuffers,
                          pipeline::Holder& pipelineHolder,
                          std::vector<vk::Framebuffer> framebuffers, vk::Extent2D& extent,
                          SecondaryBuffers&&... secondaryBuffers)
{
  for (auto i = 0; i < commandBuffers.size(); ++i) {
    auto currentSecondaryBuffers = getElementsAtIndex<vk::CommandBuffer>(
        i, std::forward<SecondaryBuffers>(secondaryBuffers)...);
    recordPrimaryBuffer<BufferUsage>(commandBuffers[i].get(), currentSecondaryBuffers,
                                     extent, framebuffers[i], pipelineHolder);
  }
}

template <vk::CommandBufferLevel BufferLevel>
auto makeBuffers(vk::Device& device, vk::CommandPool& commandPool, size_t size)
{
  std::vector<vk::UniqueCommandBuffer> result;
  result.reserve(size);
  vk::CommandBufferAllocateInfo allocateInfo{commandPool, BufferLevel,
                                             static_cast<uint32_t>(size)};
  return device.allocateCommandBuffersUnique(allocateInfo);
}

struct Holder {
  vk::UniqueCommandPool pool;

  std::vector<vk::UniqueCommandBuffer> buffers;
  std::vector<vk::UniqueCommandBuffer> sceneBuffers;

  // Holder(vk::UniqueCommandPool pool);
  Holder(vk::Device& device, vk::CommandPoolCreateFlags commandPoolType,
         uint32_t graphicsFamilyIndex, pipeline::Holder& pipelineHolder,
         std::size_t framebufferCount, vk::Extent2D& extent);
};

} // namespace gh::detail::command