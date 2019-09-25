#include "rendering.h"

#include <algorithm>
#include <type_traits>

#include <glm/gtc/matrix_transform.hpp>

#include "common.h"

namespace gh::detail {

using clock = std::chrono::high_resolution_clock;

float render::UniformBufferObject::vertexCount = 0;
float render::UniformBufferObject::rowCount = 0;

namespace render {

Synchronization::Synchronization(vk::Device& device, size_t count)
    : imageReady(create_n<std::vector<vk::UniqueSemaphore>>(
          count,
          [device] { return device.createSemaphoreUnique(vk::SemaphoreCreateInfo()); })),
      renderFinished(create_n<std::vector<vk::UniqueSemaphore>>(
          count,
          [device] { return device.createSemaphoreUnique(vk::SemaphoreCreateInfo()); })),
      inFlight(create_n<std::vector<vk::UniqueFence>>(count, [device] {
        return device.createFenceUnique(
            vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
      }))
{
}

} // namespace render

template <typename Duration> constexpr auto getModelMatrix(Duration time)
{
  // return glm::mat4{1};
  return glm::rotate<float>(glm::mat4{1}, 0.5f * std::sin(time) * glm::radians(90.0),
                            glm::vec3{0.0, 0.0, 1.0});
}

template <typename Duration> constexpr auto getViewMatrix(Duration time)
{
  return glm::lookAt(glm::vec3{2.0, 2.0, 2.0}, glm::vec3{0.0, 0.0, 0.0},
                     glm::vec3{0.0, 0.0, -1.0});
}

template <typename FloatingPoint, typename Duration>
constexpr auto getProjectionMatrix(FloatingPoint aspect, Duration time)
{
  return glm::perspective(45.0f, aspect, 0.1f, 10.0f);
}

render::UniformBufferObject getUniformBufferData(size_t width, size_t height)
{
  return getUniformBufferData(static_cast<float>(width), static_cast<float>(height));
}

render::UniformBufferObject getUniformBufferData(float width, float height)
{
  static auto start = clock::now();
  auto aspect = width / height;

  auto now = clock::now();
  auto time =
      std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
  // std::clog << __FUNCTION__ << ": " << time << std::endl;

  auto model = getModelMatrix(time);
  auto view = getViewMatrix(time);
  auto projection = getProjectionMatrix(aspect, time);

  return render::UniformBufferObject{model, view, projection,
                                     render::UniformBufferObject::vertexCount,
                                     render::UniformBufferObject::rowCount};
}

} // namespace gh::detail