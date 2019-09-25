#include "debugutils.h"

namespace gh::detail::debug {

bool validationLayersSupported() {
  auto layers = vk::enumerateInstanceLayerProperties();
  std::vector<std::string> layerNames(layers.size());
  for (auto layer : layers) {
    layerNames.emplace_back(layer.layerName);
  }
  for (auto layer : validationLayers) {
    if (std::find(layerNames.begin(), layerNames.end(), std::string(layer)) ==
        layerNames.end()) {
      return false;
    }
  }
  return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
  std::cerr << "Validation layer: "
            << vk::to_string(vk::DebugUtilsMessageTypeFlagsEXT(messageType))
            << vk::to_string(vk::DebugUtilsMessageSeverityFlagsEXT(messageSeverity))
            << std::endl
            << "  " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

DebugInfo::DebugInfo(vk::Instance &instance) : instance(instance) {
#ifndef NDEBUG
  if (!validationLayersSupported()) {
    std::clog << "Validation layers not supported" << std::endl;
    return;
  }
  auto createInfo = static_cast<VkDebugUtilsMessengerCreateInfoEXT>(
      vk::DebugUtilsMessengerCreateInfoEXT{
          vk::DebugUtilsMessengerCreateFlagsEXT(),
          vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
              vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
          // ~vk::DebugUtilsMessageSeverityFlagsEXT(),
          ~vk::DebugUtilsMessageTypeFlagsEXT(), debugCallback, nullptr});
  auto createFunction = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
  assert(createFunction != nullptr);

  VkDebugUtilsMessengerEXT tmp;
  createFunction(instance, &createInfo, nullptr, &tmp);
  callback = tmp;
#endif
}

DebugInfo::~DebugInfo() {
#ifndef NDEBUG
  if (!validationLayersSupported())
    return;
  auto destroyFunction = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
  if (destroyFunction == nullptr) {
    std::clog << "Failed to load vkDestroyDebugUtilsMessengerEXT: debugging "
                 "will not be properly cleaned up"
              << std::endl;
    return;
  }
  destroyFunction(instance, callback, nullptr);
#endif
}

} // namespace gh::detail::debug