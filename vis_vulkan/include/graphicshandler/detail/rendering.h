#pragma once

#include <chrono>
#include <complex>
#include <numeric>
#include <type_traits>

#include <vulkan/vulkan.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace gh::detail {
namespace render {

struct Synchronization {
  std::vector<vk::UniqueSemaphore> imageReady;
  std::vector<vk::UniqueSemaphore> renderFinished;

  std::vector<vk::UniqueFence> inFlight;

  Synchronization(vk::Device& device, std::size_t count);
};

template <typename T> glm::vec2 vectorize(std::complex<T> number)
{
  return glm::vec2(static_cast<float>(number.real()), static_cast<float>(number.imag()));
}

// Specify a descriptor layout during pipeline creation
// Allocate a descriptor set from a descriptor pool
// Bind the descriptor set during rendering
struct UniformBufferObject {
  // TODO: the model matrix could be a push constant (that is probably the most frequently
  // updated)
  glm::mat4 model;
  glm::mat4 view;
  glm::mat4 projection;

  float vCount;
  float rCount;
  // ehh
  float otherValues[6];
  // ehh
  static float vertexCount;
  static float rowCount;
};

struct Vertex {
  glm::vec2 data;

  template <typename T>
  Vertex(std::complex<T> number = std::complex<T>(static_cast<T>(0), static_cast<T>(0)))
      : data(vectorize(number))
  {
  }

  template <uint32_t Binding, uint32_t Location> static auto getAttributeDescription()
  {
    return vk::VertexInputAttributeDescription{
        Location, Binding, vk::Format::eR32G32Sfloat, offsetof(Vertex, data)};
  }
};

} // namespace render

template <uint32_t Binding, typename T> auto getBindingDescription()
{
  return vk::VertexInputBindingDescription{Binding, sizeof(T)};
}

// UniformBufferObject getUniformBufferData(size_t width, size_t height);
render::UniformBufferObject getUniformBufferData(float width, float height);

} // namespace gh::detail