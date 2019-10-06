#ifndef ZAMT_MODULE_VISUALIZATION_VULKAN_H_
#define ZAMT_MODULE_VISUALIZATION_VULKAN_H_

#include "zamt/core/Module.h"

#include "zamt/core/CLIParameters.h"
#include "zamt/core/Log.h"

namespace zamt {

class ModuleCenter;

class VulkanVisualizer : public Module {
 public:
  VulkanVisualizer(int argc, const char* const* argv);
  ~VulkanVisualizer() = default;

  void Initialize(const ModuleCenter* moduleCenter);
 private:

  void Shutdown(int exit_code);

  void PrintHelp();

  CLIParameters cli;
  Log logger;
};

}  // namespace zamt

#endif  // ZAMT_MODULE_VISUALIZATION_VULKAN_H_
