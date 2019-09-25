#include "graphicshandler.h"

#include <chrono>
#include <iostream>

#include "common.h"
#include "debugutils.h"
#include "guihandler.h"
#include "vulkanimpl.h"
#include "windowhandler.h"

constexpr auto FRAMES_PER_SEC = 60;

std::vector<const char*> requiredExtensions(const WindowHandler& windowHandler)
{
  auto result = windowHandler.getGLFWExtensions();
#ifndef NDEBUG
  result.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
  return result;
}

template <FileType type>
auto getValues(const std::vector<ShaderDirectoryContents>& shaders)
{
  std::vector<std::filesystem::path> result;
  result.reserve(shaders.size());
  for (auto& shader : shaders) {
    try {
      result.emplace_back(shader.at(type));
    }
    catch (...) {
      // if I recall correctly map::at can throw, but map::operator[] is not const...
    }
  }
  return result;
}

GraphicsHandler::GraphicsHandler(const std::string& appName, WindowHandler& windowHandler,
                                 std::size_t sampleSize, std::size_t displayHistory,
                                 std::vector<ShaderDirectoryContents> shaders)
    : pimpl(std::make_unique<gh::detail::HandlerImpl>(
          appName, windowHandler, shaders, requiredExtensions(windowHandler),
          validationLayers, sampleSize, displayHistory)),
      guiHandler(std::make_unique<GuiHandler>(windowHandler, *pimpl,
                                              getValues<FileType::directory>(shaders)))
{
  pimpl->setGuiHandler(*guiHandler);
}

GraphicsHandler::~GraphicsHandler() = default;

void GraphicsHandler::drawResults(result_t results)
{
  std::lock_guard lock(dataAddMutex);
  // std::clog << __FUNCTION__ << std::endl;
  pimpl->addData(std::move(results));
}

void GraphicsHandler::startDrawing()
{
  std::clog << __FUNCTION__ << std::endl;
  drawing = true;
  drawingThread = std::thread{[this] {
    while (drawing) {
      pimpl->drawFrame();
      using namespace std::chrono_literals;
      // std::this_thread::sleep_for(1000ms/FRAMES_PER_SEC);
      // TODO: fix fps?
    }
  }};
}

void GraphicsHandler::endDrawing()
{
  drawing = false;
  std::clog << __FUNCTION__ << std::endl;
  drawingThread.join();
}
