#include "zamt/vis_vulkan/VulkanVisualizer.h"

#include "zamt/core/Core.h"
#include "zamt/core/Log.h"
#include "zamt/core/ModuleCenter.h"

#include "zamt/vis_gtk/Visualization.h"

namespace zamt {

VulkanVisualizer::VulkanVisualizer(int argc, const char* const* argv)
    : cli(argc, argv), logger("vis_vulkan", cli) {
  logger.LogMessage("Starting...");
  if (cli.HasParam("-h")) {
    PrintHelp();
    return;
  }
}

void VulkanVisualizer::Initialize(const ModuleCenter* moduleCenter) {
  auto id = static_cast<int>(moduleCenter->GetId<VulkanVisualizer>());
  auto& vis = moduleCenter->Get<Visualization>();
  vis.OpenWindow("asd", 123, 234, id);
}

void VulkanVisualizer::PrintHelp() {
  Log::Print("ZAMT Visualization Module using Vulkan");
  // Log::Print(" -list");
}

}  // namespace zamt
