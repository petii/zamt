#pragma once

#include <any>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

class WindowHandler {

  std::any window;

  bool shouldClose() const;
public:
  WindowHandler(const std::string& name, int width, int height);

  ~WindowHandler();
  std::vector<const char*> getGLFWExtensions() const;
  std::any createSurface(std::any instance);

  std::tuple<int, int> getFrameBufferSize() const;
  std::any getNativeWindow() const;

  void mainloop();
};
