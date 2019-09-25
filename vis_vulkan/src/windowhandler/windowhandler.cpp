#include "windowhandler.h"

#include <thread>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

void glfwErrorCallback(int error, const char* description)
{
  std::cerr << "GLFW Error: " << error << " : " << description << std::endl;
}

WindowHandler::WindowHandler(const std::string& name, int width, int height)
{
  if (!glfwInit()) {
    throw std::runtime_error("Failed to initialize GLFW");
  }
#ifndef NDEBUG
  glfwSetErrorCallback(glfwErrorCallback);
#endif
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
  window = glfwCreateWindow(width, height, name.c_str(), nullptr, nullptr);
  if (!window.has_value()) {
    throw std::runtime_error("Failed to create window");
  }
}

WindowHandler::~WindowHandler()
{
  glfwDestroyWindow(std::any_cast<GLFWwindow*>(window));
  glfwTerminate();
}

bool WindowHandler::shouldClose() const
{
  return static_cast<bool>(glfwWindowShouldClose(std::any_cast<GLFWwindow*>(window)));
}

std::vector<const char*> WindowHandler::getGLFWExtensions() const
{
  uint32_t glfwExtensionCount = 0;
  const char** glfwExtensions;
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  return std::vector<const char*>(glfwExtensions, glfwExtensions + glfwExtensionCount);
}

std::any WindowHandler::createSurface(std::any instance)
{
  // TODO: fuck me
  auto vulkanInstance = std::any_cast<VkInstance>(instance);
  VkSurfaceKHR surface;
  glfwCreateWindowSurface(vulkanInstance, std::any_cast<GLFWwindow*>(window), nullptr,
                          &surface);
  return surface;
}

std::tuple<int, int> WindowHandler::getFrameBufferSize() const
{
  int width, height;
  glfwGetFramebufferSize(std::any_cast<GLFWwindow*>(window), &width, &height);
  return std::make_tuple(width, height);
}

void WindowHandler::mainloop()
{
  while (!shouldClose()) {
    glfwPollEvents();
    using namespace std::chrono_literals;
    std::this_thread::sleep_for(10ms);
  }
}

std::any WindowHandler::getNativeWindow() const {
  return window;
}