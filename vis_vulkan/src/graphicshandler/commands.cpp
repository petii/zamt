#include "commands.h"

namespace gh::detail::command {

Holder::Holder(vk::Device& device, vk::CommandPoolCreateFlags commandPoolType,
               uint32_t graphicsFamilyIndex, pipeline::Holder& pipelineHolder,
               std::size_t framebufferCount, vk::Extent2D& extent)
    : pool(device.createCommandPoolUnique(
          vk::CommandPoolCreateInfo{commandPoolType, graphicsFamilyIndex})),
      buffers(makeBuffers<vk::CommandBufferLevel::ePrimary>(device, pool.get(),
                                                            framebufferCount)),
      sceneBuffers(makeBuffers<vk::CommandBufferLevel::eSecondary>(device, pool.get(),
                                                                   framebufferCount))
{
  // recordBuffers<vk::CommandBufferUsageFlagBits::eSimultaneousUse>(
  //     this->buffers, pipelineHolder, framebuffers, extent, buffers, listSize);
}

void recordSceneSecondaryBuffer(vk::CommandBuffer& buffer,
                                pipeline::Holder& pipelineHolder,
                                vk::DescriptorSet& descriptorSet, buffer::Holder& buffers,
                                vk::Framebuffer& framebuffer)
{
  vk::CommandBufferInheritanceInfo inheritanceInfo{pipelineHolder.renderPass.get(), 0,
                                                   framebuffer};
  buffer.begin(vk::CommandBufferBeginInfo{
      vk::CommandBufferUsageFlagBits::eRenderPassContinue, &inheritanceInfo});
  {
    buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipelineHolder.pipeline.get());

    buffer.bindVertexBuffers(0, {buffers.vertex.get()}, {0});

    buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                              pipelineHolder.pipelineLayout.get(), 0, {descriptorSet},
                              {});

    buffer.bindIndexBuffer(buffers.index.get(), 0, vk::IndexType::eUint32);

    buffer.drawIndexed(buffers.indexCount, 1, 0, 0, 0);
  }
  buffer.end();
}

void recordSceneSecondaryBuffers(std::vector<vk::UniqueCommandBuffer>& buffers,
                                 pipeline::Holder& pipelineHolder,
                                 buffer::Holder& bufferHolder,
                                 std::vector<vk::Framebuffer> framebuffers)
{
  for (auto i = 0; i < buffers.size(); ++i) {
    recordSceneSecondaryBuffer(buffers[i].get(), pipelineHolder,
                               pipelineHolder.descriptorSets[i], bufferHolder,
                               framebuffers[i]);
  }
}

} // namespace gh::detail::command
