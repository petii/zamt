#pragma once

#include <atomic>
#include <complex>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "shader_directories.h"

class WindowHandler;
class GuiHandler;

namespace gh::detail {
struct HandlerImpl;
}

class GraphicsHandler {

  std::unique_ptr<gh::detail::HandlerImpl> pimpl;

  std::atomic_bool drawing;
  std::thread drawingThread;

  std::mutex dataAddMutex;

  std::unique_ptr<GuiHandler> guiHandler;

public:
  using result_t = std::vector<std::complex<float>>;

  GraphicsHandler(const std::string&, WindowHandler& windhowHandler,
                  std::size_t sampleSize, std::size_t displayHistory,
                  std::vector<ShaderDirectoryContents> shaders);
  ~GraphicsHandler();

  void startDrawing();
  void endDrawing();

  void drawResults(result_t results);
};