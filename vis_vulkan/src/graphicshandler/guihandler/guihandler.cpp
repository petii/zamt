#include "guihandler.h"

#include <any>
#include <chrono>
#include <iostream>
#include <map>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <vulkan/vulkan.hpp>

#include "graphicshandler.h"
#include "vulkanobjects.h"
#include "windowhandler.h"

#include "common.h"

namespace gui::detail {

// namespace /*mouseGlobals*/{
// std::array<bool,GLFW_MOUSE_BUTTON_LAST> mouseButtonsPressed;
// }

void scrollCallback(GLFWwindow* w, double xoffset, double yoffset)
{
  auto io = ImGui::GetIO();
  io.MouseWheelH += xoffset;
  io.MouseWheel += yoffset;
}

void keyCallback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
  ImGuiIO& io = ImGui::GetIO();
  if (action == GLFW_PRESS)
    io.KeysDown[key] = true;
  if (action == GLFW_RELEASE)
    io.KeysDown[key] = false;

  // Modifiers are not reliable across systems
  io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
  io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
  io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
  io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

void charCallback(GLFWwindow* w, unsigned int c)
{
  ImGuiIO& io = ImGui::GetIO();
  io.AddInputCharacter(static_cast<ImWchar>(c));
}

void setupGlfw(GLFWwindow* window, GuiHandler& gui)
{
  auto& io = ImGui::GetIO();

  // key mapping
  io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
  io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
  io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
  io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
  io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
  io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
  io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
  io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
  io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
  io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
  io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
  io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
  io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
  io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
  io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;

  glfwSetKeyCallback(window, keyCallback);
  glfwSetCharCallback(window, charCallback);
  // mouse handling
  glfwSetScrollCallback(window, scrollCallback);
  // glfwSetMouseButtonCallback(window, [](auto window, auto button, auto action, auto
  // mods){
  //   if (action == GLFW_PRESS) {
  //     gui::detail::mouseButtonsPressed[button] = true;
  //   }
  // });

  glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, true);
}

} // namespace gui::detail

GuiHandler::GuiHandler(WindowHandler& windowHandler, gh::detail::HandlerImpl& vulkanImpl,
                       std::vector<std::filesystem::path> shaderDirs)
    : wh(windowHandler)
{
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  auto& io = ImGui::GetIO();

  auto [width, height] = windowHandler.getFrameBufferSize();
  io.DisplaySize = ImVec2{static_cast<float>(width), static_cast<float>(height)};
  std::clog << "ImGui::GetIO().DisplaySize: " << io.DisplaySize.x << "x"
            << io.DisplaySize.y << std::endl;

  gui::detail::setupGlfw(std::any_cast<GLFWwindow*>(windowHandler.getNativeWindow()),
                         *this);

  objects = std::make_unique<gui::detail::VulkanObjects>(vulkanImpl);

  ImGui::StyleColorsDark();

  objects->setupFonts();

  for (auto& dir : shaderDirs) {
    shaderNames.emplace_back(dir.string());
  }
}

void GuiHandler::startFrame()
{
  // using clock = std::chrono::high_resolution_clock;
  // static clock::time_point previousFrame = clock::now();
  static double previousFrame = glfwGetTime();
  ImGui::NewFrame();
  auto window = std::any_cast<GLFWwindow*>(wh.getNativeWindow());
  auto& io = ImGui::GetIO();
  // io.DeltaTime = std::chrono::duration<>
  io.DeltaTime = static_cast<float>(glfwGetTime() - previousFrame);

  // mouse
  // auto previousMousePosition = io.MousePos;
  // position
  if (glfwGetWindowAttrib(window, GLFW_FOCUSED)) {
    double cursorX, cursorY;
    glfwGetCursorPos(window, &cursorX, &cursorY);
    io.MousePos = ImVec2{static_cast<float>(cursorX), static_cast<float>(cursorY)};
  }
  // button press
  for (auto i = 0; i < IM_ARRAYSIZE(io.MouseDown); ++i) {
    // io.MouseDown[i] = gui::detail::mouseButtonsPressed[i] | glfwGetMouseButton(window,
    // i); gui::detail::mouseButtonsPressed[i] = false;
    io.MouseDown[i] = glfwGetMouseButton(window, i);
  }

  showWindows();
}

void GuiHandler::showWindows()
{
#ifndef NDEBUG
  ImGui::ShowDemoWindow();
#endif
  ImGui::Begin("Customization");
  {
    if (ImGui::CollapsingHeader("Rasterization", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (ImGui::RadioButton("Fill", &rasterModeInt, 0)) {
        rasterChange(vk::PolygonMode::eFill);
      }
      // ImGui::SameLine();
      if (ImGui::RadioButton("Line", &rasterModeInt, 1)) {
        rasterChange(vk::PolygonMode::eLine);
      }
      // ImGui::SameLine();
      if (ImGui::RadioButton("Point", &rasterModeInt, 2)) {
        rasterChange(vk::PolygonMode::ePoint);
      }
    }
    if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
      if (ImGui::RadioButton("Linear", &scaleModeInt, 0)) {
        scaleChange(ScaleType::linear);
      }
      // ImGui::SameLine();
      if (ImGui::RadioButton("Exponential", &scaleModeInt, 1)) {
        scaleChange(ScaleType::exponential);
      }
      // ImGui::SameLine();
      if (ImGui::RadioButton("Logarithmic", &scaleModeInt, 2)) {
        scaleChange(ScaleType::logarithmic);
      }
    }

    if (ImGui::CollapsingHeader("Shader", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::BeginChild("Scrolling");
      {
        static std::string selected = shaderNames.front();
        for (auto& name : shaderNames) {
          if (ImGui::Selectable(name.c_str(), selected == name)) {
            selected = name;
            shaderChange(selected);
          }
        }
        // if (ImGui::Selectable("Shader1", selected == 1)) {
        //   selected = 1;
        // }
        // if (ImGui::Selectable("Shader2", selected == 2)) {
        //   selected = 2;
        // }
      }
      ImGui::EndChild();
    }
    ImGui::End();
  }

}

void GuiHandler::render(std::size_t frameIndex)
{
  ImGui::Render();
  auto drawData = ImGui::GetDrawData();
  if (drawData == nullptr || drawData->TotalVtxCount == 0) {
    // std::clog << __FUNCTION__ << "empty DrawData, returning" << std::endl;
    objects->recordEmptyCommandBuffer(frameIndex);
    return;
  }
  objects->buffers.uploadDrawData(*drawData);

  objects->recordGuiSecondaryBuffer(frameIndex, *drawData);
}

GuiHandler::~GuiHandler() = default;

std::vector<vk::CommandBuffer> GuiHandler::getCommandBuffers()
{
  return getHandles<vk::CommandBuffer>(objects->guiBuffers);
}
