#pragma once

#include <complex>
#include <random>
#include <unordered_map>
#include <vector>

#include <boost/circular_buffer.hpp>
#include <vulkan/vulkan.hpp>

#include "buffers.h"
#include "commands.h"
#include "debugutils.h"
#include "images.h"
#include "pipelines.h"
#include "rendering.h"
#include "shader_directories.h"
#include "shaders.h"
#include "swapchains.h"

#define APP_VERSION VK_MAKE_VERSION(0, 1, 0)

class WindowHandler;
class GuiHandler;

namespace gh::scale {

namespace detail {

template <typename T, typename Function>
constexpr auto scaleBase(std::complex<T>&& number, Function function)
{
  auto length = std::abs(number);
  return number * function(length);
}

template<typename T>
constexpr auto euler() {
  return static_cast<T>(2.7182818284590452353602874);
}

} // namespace detail

template <typename T> constexpr auto linear(T&& number) { return number; }


template <typename T> auto exponential(T&& number)
{
  return detail::scaleBase(std::forward<T>(number), [](auto t) { return std::pow(detail::euler<T>(),t); });
}

template <typename T> auto logarithmic(T&& number)
{
  return detail::scaleBase(std::forward<T>(number), [](auto t) { return std::log(t); });
}

} // namespace gh::scale

namespace gh::detail {

struct HandlerImpl {
  vk::PolygonMode polygonMode = vk::PolygonMode::eFill;

  const vk::ApplicationInfo appInfo;

  std::size_t historySize = 10;

  boost::circular_buffer<render::Vertex> vertices;
  render::Vertex* mappedVertices;

  vk::UniqueInstance instance;

  debug::DebugInfo debugInfo;

  vk::SurfaceKHR surface;

  vk::PhysicalDevice physicalDevice;
  std::unordered_map<vk::QueueFlagBits, int> queueFamilyIndices;

  vk::UniqueDevice device;

  vk::Queue graphicsQueue;

  swapchain::Holder swapchainHolder;
  image::Holder images;

  using ShadersType = std::vector<shader::Holder>;
  ShadersType shaders;
  ShadersType::iterator currentShaderIt;

  buffer::Holder bufferHolder;

  pipeline::Holder pipelineHolder;

  std::vector<vk::UniqueFramebuffer> framebuffers;

  command::Holder commandHolder;

  render::Synchronization sync;

  HandlerImpl(const std::string&, WindowHandler&,
              std::vector<ShaderDirectoryContents>& shaderDirs,
              std::vector<const char*> extensions, std::vector<const char*> layers = {},
              std::size_t vertexCount = 1024, std::size_t history = 10);

  ~HandlerImpl();

  void setGuiHandler(GuiHandler&);

  void addData(std::vector<std::complex<float>> data);
  void drawFrame();

private:
  WindowHandler& windowHandler;
  GuiHandler* guiHandler;

  void onOutOfDateSwapchain(int width, int height);

  void updateVertices();
  void updateIndices();

  using ScaleFunctionType = std::function<std::complex<float>(std::complex<float>&&)>;
  ScaleFunctionType scaleFunction = scale::linear<ScaleFunctionType::result_type>;
};

} // namespace gh::detail
