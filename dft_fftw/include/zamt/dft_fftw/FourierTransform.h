#ifndef ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
#define ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_

#include <complex>
#include <memory>
#include <vector>

#include "CLIParameters.h"
#include "Core.h"
#include "Log.h"
#include "Module.h"
#include "ModuleCenter.h"
#include "Scheduler.h"

#include "LiveAudio.h"

namespace zamt {

namespace dft_fftw {
namespace internal {
struct SubscriptionInfo {
  Scheduler::SourceId source_id;
  int subscription_id;
};

struct FFTW_Wrapper;
}  // namespace internal

class FourierTransform : public Module {
  std::string module_name;

 public:
  FourierTransform(int argc, const char* const* argv);
  ~FourierTransform() = default;

  void Initialize(const ModuleCenter* module_center);

 private:
  void PrintHelp() { Log::Print("ZAMT Discrete Fourier-transform with FFTW"); }

  std::atomic_bool should_run_dft{false};
  dft_fftw::internal::SubscriptionInfo subscription;

  CLIParameters cli;
  Log log;
  Scheduler* scheduler = nullptr;

  std::unique_ptr<internal::FFTW_Wrapper> worker = nullptr;
};

}  // namespace dft_fftw
}  // namespace zamt

#endif  // ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
