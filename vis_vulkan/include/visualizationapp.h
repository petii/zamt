#pragma once

#include "audiohandler.h"
#include "fourierhandler.h"
#include "graphicshandler.h"
#include "windowhandler.h"

#include "shader_directories.h"

class VisualizationApp {
public:
  static constexpr auto WIDTH = 1000;
  static constexpr auto HEIGHT = 600;

  static constexpr auto APPLICATION_NAME = "Audiovisualization with Vulkan";

  // static constexpr auto SAMPLE_SIZE = (1024 * 16) - 3;
  static constexpr auto SAMPLE_SIZE = (1024 * 2);
  // static constexpr auto SAMPLE_SIZE = (1024 * 4);
  // static constexpr auto SAMPLE_SIZE = (512);
  // static constexpr auto SAMPLE_SIZE = (32);
  // static constexpr auto HISTORY_SIZE = 16;
  // static constexpr auto SAMPLE_SIZE = (16);
  // static constexpr auto HISTORY_SIZE = 8;
  static constexpr auto HISTORY_SIZE = 64;

  static constexpr auto RESULT_SIZE = SAMPLE_SIZE / 2;
private:
  FourierHandler fourierHandler;
  AudioHandler audioHandler;
  WindowHandler windowHandler;
  GraphicsHandler graphicsHandler;
  // triple buffering megnezese
  // osszehasonlitas

  // teszteles helyett meresek
  // tablazatok
  // laptop, asztali gep osszehasonlitasa
public:
  VisualizationApp(bool monitoring, double overlap,
                   std::vector<ShaderDirectoryContents> shaders);

  void run();
};
