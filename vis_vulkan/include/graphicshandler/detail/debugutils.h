#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <vector>

#include <vulkan/vulkan.hpp>

const std::vector<const char *> validationLayers = {
#ifndef NDEBUG
    "VK_LAYER_LUNARG_standard_validation"
#endif
};

namespace gh::detail::debug {

bool validationLayersSupported();

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);

struct DebugInfo {

  vk::DebugUtilsMessengerEXT callback;
  vk::Instance &instance;

  DebugInfo(vk::Instance &instance);
  ~DebugInfo();
};

} // namespace gh::detail::debug