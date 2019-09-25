#include "buffers.h"

#include <iostream>
#include <utility>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace gh::detail {

std::uint32_t findMemoryType(vk::PhysicalDevice& physicalDevice, uint32_t typeFilter,
                             vk::MemoryPropertyFlags properties)
{
  auto memoryProperties = physicalDevice.getMemoryProperties();
  auto from = std::begin(memoryProperties.memoryTypes);
  auto to = std::end(memoryProperties.memoryTypes);
  auto result = std::find_if(from, to, [&](auto& property) {
    auto index = std::distance(std::find(from, to, property), from);
    return typeFilter & (1 << index) // what
           && property.propertyFlags & properties;
  });
  return std::distance(result, from);
}

namespace buffer {

vk::UniqueDeviceMemory allocateMemory(vk::PhysicalDevice& physicalDevice,
                                      vk::Device& device, vk::MemoryRequirements vertex,
                                      vk::MemoryRequirements index,
                                      vk::MemoryPropertyFlags properties)
{
  // auto vertexMemoryRequirements = device.getBufferMemoryRequirements(vertex);
  // auto indexMemoryRequirements = device.getBufferMemoryRequirements(index);
  auto memorySize = vertex.size + index.size;
  auto memoryTypeIndex = findMemoryType(
      physicalDevice, vertex.memoryTypeBits | index.memoryTypeBits, properties);

  std::clog << __FUNCTION__ << ": memory type index: " << memoryTypeIndex
            << " | allocating " << memorySize << " bits = " << vertex.size
            << " (vertex) + " << index.size << " (index)" << std::endl;

  return device.allocateMemoryUnique(vk::MemoryAllocateInfo{memorySize, memoryTypeIndex});
}

vk::UniqueDeviceMemory allocateAndBindMemory(vk::PhysicalDevice& physicalDevice,
                                             vk::Device& device, vk::Buffer& vertex,
                                             vk::Buffer& index,
                                             vk::MemoryPropertyFlags properties)
{
  auto vertexMemoryRequirements = device.getBufferMemoryRequirements(vertex);
  auto indexMemoryRequirements = device.getBufferMemoryRequirements(index);
  auto memory = allocateMemory(physicalDevice, device, vertexMemoryRequirements,
                               indexMemoryRequirements, properties);
  device.bindBufferMemory(vertex, memory.get(), 0);
  device.bindBufferMemory(index, memory.get(), vertexMemoryRequirements.size);

  std::clog << __FUNCTION__ << ": memory binding info:\n  vertex alignment: "
            << vertexMemoryRequirements.alignment << std::endl
            << "  index alignment: " << indexMemoryRequirements.alignment << std::endl;

  return std::move(memory);
}

vk::UniqueDeviceMemory allocateAndBindMemory(vk::PhysicalDevice& physicalDevice,
                                             vk::Device& device,
                                             std::vector<vk::Buffer> uniform,
                                             vk::MemoryPropertyFlags properties)
{
  if (uniform.empty()) {
    return vk::UniqueDeviceMemory{nullptr};
  }
  std::clog << __FUNCTION__ << ": allocated memory info: " << std::endl;
  vk::DeviceSize size = 0;
  std::vector<vk::MemoryRequirements> memoryReqs;
  for (auto& buffer : uniform) {
    auto req = device.getBufferMemoryRequirements(buffer);
    size += req.size;
    std::clog << "     " << req.size << " bits" << std::endl;
    memoryReqs.emplace_back(req);
  }
  std::clog << "sum: " << size << std::endl;
  auto memory = device.allocateMemoryUnique(vk::MemoryAllocateInfo{
      size,
      findMemoryType(physicalDevice, memoryReqs.front().memoryTypeBits, properties)});
  auto offset = 0;
  for (auto& [buffer, req] : combine_vectors(uniform, memoryReqs)) {
    device.bindBufferMemory(buffer, memory.get(), offset);
    offset += req.size;
  }

  return std::move(memory);
}

vk::UniqueDeviceMemory allocateAndBindMemory(vk::PhysicalDevice& physicalDevice,
                                             vk::Device& device, vk::Buffer buffer,
                                             vk::MemoryPropertyFlags properties)
{
  auto memoryReqs = device.getBufferMemoryRequirements(buffer);
  auto memory = device.allocateMemoryUnique(vk::MemoryAllocateInfo{
      memoryReqs.size,
      findMemoryType(physicalDevice, memoryReqs.memoryTypeBits, properties)});
  std::clog << __FUNCTION__ << ": allocated memory info: buffer: " << buffer << " | "
            << memoryReqs.size << " bits" << std::endl;
  device.bindBufferMemory(buffer, memory.get(), 0);
  return std::move(memory);
}

void Holder::writeUniformBufferData(unsigned index, render::UniformBufferObject data)
{
#ifdef DEBUG_PROJECTION_MATRICES
  std::clog << __FUNCTION__ << ": to " << index << ", offset(" << offset << ") , size("
            << sizeof(data) << " / " << sizeof(UniformBufferObject) << ")" << std::endl;
  std::clog << "  model: " << glm::to_string(data.model) << std::endl
            << "  view: " << glm::to_string(data.view) << std::endl
            << "  proj: " << glm::to_string(data.projection) << std::endl;
#endif
  auto* mem = reinterpret_cast<render::UniformBufferObject*>(uniformMapped);
  mem += index;
  std::memcpy(mem, &data, uniformBufferRequirements.size);
}

Holder::~Holder() { device.unmapMemory(uniformMemory.get()); }

} // namespace buffer

} // namespace gh::detail
