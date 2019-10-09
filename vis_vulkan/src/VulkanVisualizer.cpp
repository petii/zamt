#include "zamt/vis_vulkan/VulkanVisualizer.h"

#include <type_traits>

#include "zamt/core/Core.h"
#include "zamt/core/Log.h"
#include "zamt/core/ModuleCenter.h"

#include "zamt/liveaudio_pulse/LiveAudio.h"

#include "zamt/vis_gtk/Visualization.h"

#include "zamt/dft_fftw/FourierTransform.h"

namespace zamt {

VulkanVisualizer::VulkanVisualizer(int argc, const char* const* argv)
    : cli(argc, argv), logger("vis_vulkan", cli) {
  logger.LogMessage("Starting...");
  if (cli.HasParam("-h")) {
    PrintHelp();
    return;
  }
}

#define BOOLCSTR(expr) (expr ? #expr " = true" : #expr " = false")

void VulkanVisualizer::Initialize(const ModuleCenter* moduleCenter) {
  // using namespace string_literals;

  // register ourselves by initializing a ModuleStub template
  auto id = static_cast<int>(moduleCenter->GetId<VulkanVisualizer>());

  // get the core module handle (mainly for the scheduler)
  auto& core = moduleCenter->Get<Core>();
  logger.LogMessage(core.kModuleLabel);
  // auto& scheduler = core.scheduler();
  // logger.LogMessage(std::to_string(scheduler.GetNumberOfWorkers()).c_str());

  // get the pulseaudio module id
  auto audio = moduleCenter->GetId<LiveAudio>();
  logger.LogMessage(std::to_string(audio).c_str());

  // get the gtk module handle
  auto& vis = moduleCenter->Get<Visualization>();
  vis.OpenWindow("asd", 123, 234, id);

  // now the fun part
  auto& ft = moduleCenter->Get<FourierTransform<int>>();
  logger.LogMessage(
      std::to_string(
          moduleCenter->GetId<std::remove_reference_t<decltype(ft)>>())
          .c_str());
  auto& ft2 = moduleCenter->Get<FourierTransform<float>>();
  logger.LogMessage(
      std::to_string(
          moduleCenter->GetId<std::remove_reference_t<decltype(ft2)>>())
          .c_str());
}

void VulkanVisualizer::PrintHelp() {
  Log::Print("ZAMT Visualization Module using Vulkan");
  // Log::Print(" -list");
}

}  // namespace zamt
