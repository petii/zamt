#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include <boost/signals2.hpp>

class WindowHandler;

namespace vk {
class CommandBuffer;
enum class PolygonMode;
} // namespace vk

namespace gh::detail {
struct HandlerImpl;
}

namespace gui::detail {
struct VulkanObjects;
}

enum class GuiAction { shaderChange, rasterizationChange, scaleChange };

enum class ScaleType { linear, exponential, logarithmic };
constexpr auto to_string(ScaleType t)
{
  switch (t) {
  case ScaleType::linear:
    return "linear";
  case ScaleType::exponential:
    return "exponential";
  case ScaleType::logarithmic:
    return "logarithmic";
  }
}

class GuiHandler {
  std::unique_ptr<gui::detail::VulkanObjects> objects;

  int rasterModeInt = 0;

  int scaleModeInt = 0;

  using ShaderChangeType = boost::signals2::signal<void(std::string)>;
  ShaderChangeType shaderChange;

  using RasterizationChangeType = boost::signals2::signal<void(vk::PolygonMode)>;
  RasterizationChangeType rasterChange;

  using ScaleChangeType = boost::signals2::signal<void(ScaleType)>;
  ScaleChangeType scaleChange;

public:
  // initialize:
  //  - setup imgui context
  //  - setup platform/renderer bindings
  //  - setup style
  //  - upload fonts (optional, has default)
  GuiHandler(WindowHandler& windowHandler, gh::detail::HandlerImpl&,
             std::vector<std::filesystem::path> shaderDirs);
  ~GuiHandler();
  // upload display size, time, mouse stuff, keyboard stuff
  void startFrame();

  void render(std::size_t frameIndex);

  void present();

  std::vector<vk::CommandBuffer> getCommandBuffers();

  boost::signals2::connection onShaderChange(ShaderChangeType::slot_type subscriber)
  {
    return shaderChange.connect(subscriber);
  }

  boost::signals2::connection onScaleChange(ScaleChangeType::slot_type subscriber)
  {
    return scaleChange.connect(subscriber);
  }

  boost::signals2::connection
  onRasterizationChange(RasterizationChangeType::slot_type subscriber)
  {
    return rasterChange.connect(subscriber);
  }

private:
  void showWindows();

  WindowHandler& wh;

  std::vector<std::string> shaderNames;
};