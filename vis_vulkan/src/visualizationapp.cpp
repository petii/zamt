#include "visualizationapp.h"

#include <chrono>
#include <iterator>
#include <numeric>
#include <thread>

#include "common.h"
#include "window_function.h"

VisualizationApp::VisualizationApp(bool monitoring, double overlap,
                                   std::vector<ShaderDirectoryContents> shaders)
    : fourierHandler(SAMPLE_SIZE),
      audioHandler(SAMPLE_SIZE, overlap,
                   [this](auto data) {
                     auto windowedData = apply_window(
                         std::move(data),
                         // lamba because g++...
                         [](auto i) { return window::hann<SAMPLE_SIZE, float>(i); });
                     auto results = fourierHandler.doTransform(std::move(windowedData));
                     graphicsHandler.drawResults(std::move(results));
                   },
                   monitoring),
      windowHandler(APPLICATION_NAME, WIDTH, HEIGHT),
      graphicsHandler(APPLICATION_NAME, windowHandler, RESULT_SIZE, HISTORY_SIZE,
                      std::move(shaders))
{
}

void VisualizationApp::run()
{
  window::setupHannTable<SAMPLE_SIZE, float>();

  audioHandler.startRecording();
  graphicsHandler.startDrawing();
  windowHandler.mainloop();
  audioHandler.stopRecording();
  graphicsHandler.endDrawing();
}
