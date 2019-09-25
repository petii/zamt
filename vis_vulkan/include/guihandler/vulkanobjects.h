#pragma once

#include <map>
#include <vector>

#include <glm/vec2.hpp>
#include <imgui.h>
#include <vulkan/vulkan.hpp>

namespace gh::detail {
struct HandlerImpl;
}

namespace gui::detail {

struct PushConstants {
  glm::vec2 scale;
  glm::vec2 translate;
};

struct Buffers {
  vk::UniqueBuffer vertexBuffer;
  vk::UniqueDeviceMemory vertexMemory;
  vk::UniqueBuffer indexBuffer;
  vk::UniqueDeviceMemory indexMemory;

  std::size_t vertexSize = 0;
  std::size_t indexSize = 0;

  ImDrawVert* mappedVertexMemory = nullptr;
  uint32_t* mappedIndexMemory = nullptr;

  Buffers(vk::PhysicalDevice&, vk::Device&);

  void uploadDrawData(ImDrawData&);
  void recreateVertexBuffer(std::size_t newSize);
  void recreateIndexBuffer(std::size_t newSize);

private:
  vk::PhysicalDevice& physicalDevice;
  vk::Device& device;
};

struct FontObjects {
  vk::UniqueSampler sampler;

  vk::UniqueImage image;
  vk::UniqueDeviceMemory imageMemory;
  vk::UniqueImageView imageView;

  void createImage(std::size_t width, std::size_t height);
  void createImageView();

  FontObjects(vk::PhysicalDevice& physicalDevice, vk::Device& device)
      : physicalDevice(physicalDevice), device(device)
  {
  }

private:
  vk::PhysicalDevice& physicalDevice;
  vk::Device& device;
};

struct VulkanObjects {
private:
  vk::PhysicalDevice& physicalDevice;
  vk::Device& device;

public:
  std::map<vk::ShaderStageFlagBits, vk::UniqueShaderModule> shaders;

  vk::UniqueDescriptorPool descriptorPool;
  vk::UniqueDescriptorSetLayout descriptorSetLayout;
  std::vector<vk::DescriptorSet> descriptorSets;

  vk::RenderPass& renderPass; // the gh::detail::pipeline::Holder object's renderpass
  vk::UniquePipelineLayout pipelineLayout;
  vk::UniquePipeline guiPipeline;

  std::vector<vk::UniqueCommandBuffer> guiBuffers;

  Buffers buffers;

  vk::UniqueBuffer uploadBuffer;
  vk::UniqueDeviceMemory uploadBufferMemory;
  vk::UniqueCommandBuffer uploadCommandBuffer;

  FontObjects fontObjects;

  VulkanObjects(gh::detail::HandlerImpl& vulkanImpl);
  ~VulkanObjects() { device.waitIdle(); }

  void setupFonts();

  void recordGuiSecondaryBuffer(std::size_t index, ImDrawData&);
  void recordEmptyCommandBuffer(std::size_t i);

private:
  gh::detail::HandlerImpl& vulkanImpl;
};

} // namespace gui::detail