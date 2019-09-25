#pragma once

#include <vector>

#include <vulkan/vulkan.hpp>

#include "common.h"
#include "rendering.h"

namespace gh::detail {

std::uint32_t findMemoryType(vk::PhysicalDevice& physicalDevice, uint32_t typeFilter,
                             vk::MemoryPropertyFlags properties);

namespace buffer {

template <vk::BufferUsageFlagBits Usage, vk::SharingMode SharingMode, typename T>
auto createBuffer(vk::Device& device, std::size_t count = 1,
                  std::vector<uint32_t> queueFamilyIndices = {})
{
  const auto size = sizeof(T) * count;
  vk::BufferCreateInfo createInfo{vk::BufferCreateFlags(),
                                  size,
                                  Usage,
                                  SharingMode,
                                  static_cast<uint32_t>(queueFamilyIndices.size()),
                                  queueFamilyIndices.data()};
  return std::move(device.createBufferUnique(createInfo));
}

vk::UniqueDeviceMemory allocateMemory(vk::PhysicalDevice& physicalDevice,
                                      vk::Device& device, vk::MemoryRequirements vertex,
                                      vk::MemoryRequirements index,
                                      vk::MemoryPropertyFlags properties);

vk::UniqueDeviceMemory allocateAndBindMemory(vk::PhysicalDevice& physicalDevice,
                                             vk::Device& device, vk::Buffer& vertex,
                                             vk::Buffer& index,
                                             vk::MemoryPropertyFlags properties);

vk::UniqueDeviceMemory allocateAndBindMemory(vk::PhysicalDevice& physicalDevice,
                                             vk::Device& device,
                                             std::vector<vk::Buffer> uniform,
                                             vk::MemoryPropertyFlags properties);

vk::UniqueDeviceMemory allocateAndBindMemory(vk::PhysicalDevice& physicalDevice,
                                             vk::Device& device, vk::Buffer buffer,
                                             vk::MemoryPropertyFlags properties);

struct Holder {

  vk::UniqueBuffer vertex;
  vk::UniqueDeviceMemory memory;
  vk::UniqueBuffer index;
  vk::UniqueDeviceMemory indexMemory;
  std::uint32_t indexCount;

  // own uniform buffer for every swapchain image
  std::vector<vk::UniqueBuffer> uniform;
  vk::UniqueDeviceMemory uniformMemory;

  vk::MemoryRequirements vertexRequirements;
  vk::MemoryRequirements indexRequirements;
  vk::MemoryRequirements uniformBufferRequirements;

  template <typename T = render::Vertex, typename Int = uint32_t>
  Holder(vk::PhysicalDevice& physicalDevice, vk::Device& device, std::size_t vertexCount,
         std::size_t indexCount, std::size_t swapchainImageCount)
      : vertex(createBuffer<vk::BufferUsageFlagBits::eVertexBuffer,
                            vk::SharingMode::eExclusive, T>(device, vertexCount)),
        index(createBuffer<vk::BufferUsageFlagBits::eIndexBuffer,
                           vk::SharingMode::eExclusive, Int>(device, indexCount)),
        indexMemory(allocateAndBindMemory(physicalDevice, device, index.get(),
                                          vk::MemoryPropertyFlagBits::eHostVisible |
                                              vk::MemoryPropertyFlagBits::eHostCoherent)),
        indexCount(indexCount),
        memory(allocateAndBindMemory(physicalDevice, device, vertex.get(),
                                     vk::MemoryPropertyFlagBits::eHostVisible |
                                         vk::MemoryPropertyFlagBits::eHostCoherent)),
        uniform(create_n<std::vector<vk::UniqueBuffer>>(
            swapchainImageCount,
            [&device]() {
              return createBuffer<vk::BufferUsageFlagBits::eUniformBuffer,
                                  vk::SharingMode::eExclusive, render::UniformBufferObject>(
                  device);
            })),
        uniformMemory(
            allocateAndBindMemory(physicalDevice, device, getHandles<vk::Buffer>(uniform),
                                  vk::MemoryPropertyFlagBits::eHostVisible |
                                      vk::MemoryPropertyFlagBits::eHostCoherent)),
        vertexRequirements(device.getBufferMemoryRequirements(vertex.get())),
        indexRequirements(device.getBufferMemoryRequirements(index.get())),
        uniformBufferRequirements(device.getBufferMemoryRequirements(uniform[0].get())),
        device(device),
        uniformMapped(device.mapMemory(uniformMemory.get(), 0, VK_WHOLE_SIZE))
  {
  }

  ~Holder();

  void writeUniformBufferData(unsigned index, render::UniformBufferObject data);

private:
  vk::Device& device;
  void* uniformMapped;
};

// TODO: check back with variadic templates sometime
// template <vk::MemoryPropertyFlagBits, typename... Args>
// auto allocateMemory(vk::PhysicalDevice& physicalDevice, vk::Device& device, Args...
// args)
// {
//   auto memorySize = ;
//   auto memoryTypeIndex = findMemoryType(std::forward<Args>(args)...);
//   auto resultMemory =
//       device.allocateMemoryUnique(vk::MemoryAllocateInfo{size, memoryTypeIndex});

//   return std::move(resultMemory);
// }

} // namespace buffer

} // namespace gh::detail
