#ifndef ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
#define ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_

#include <complex>
#include <vector>

#include "zamt/core/CLIParameters.h"
#include "zamt/core/Core.h"
#include "zamt/core/Log.h"
#include "zamt/core/Module.h"
#include "zamt/core/ModuleCenter.h"
#include "zamt/core/Scheduler.h"

#include "zamt/liveaudio_pulse/LiveAudio.h"

namespace zamt {

namespace dft_fftw {
namespace internal {
struct SubscriptionInfo {
  Scheduler::SourceId source_id;
  int subscription_id;
};
}  // namespace internal
}  // namespace dft_fftw

using namespace std::string_literals;

class FourierTransform : public Module {
  std::string module_name;

 public:
  FourierTransform(int argc, const char* const* argv)
      : module_name("dft_fftw"s),
        cli(argc, argv),
        log(module_name.c_str(), cli) {
    log.LogMessage("Starting...");
    if (cli.HasParam("-h")) {
      PrintHelp();
      return;
    }
  }
  ~FourierTransform() {}

  void Initialize(const ModuleCenter* module_center) {
    log.Message("Initialize...");
    scheduler = &module_center->Get<Core>().scheduler();
#ifdef ZAMT_MODULE_LIVEAUDIO_PULSE
    auto audio = module_center->GetId<LiveAudio>();
    int subscriptionId;
    scheduler->Subscribe(
        audio,
        [&](auto id, auto packet, auto /*time*/) {
          scheduler->ReleasePacket(id, packet);
        },
        false, subscriptionId);
#endif
    auto id = module_center->GetId<FourierTransform>();
    log.Message("audio source = ", audio, ", self id = ", id);
  }

 private:
  void PrintHelp() { Log::Print("ZAMT Discrete Fourier-transform with FFTW"); }

  // dft_fftw::internal::SubscriptionInfo subscription;

  CLIParameters cli;
  Log log;

  Scheduler* scheduler = nullptr;
};

}  // namespace zamt

#endif  // ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
