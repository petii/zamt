#ifndef ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
#define ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_

#include <complex>
#include <vector>

#include "zamt/core/CLIParameters.h"
#include "zamt/core/Log.h"
#include "zamt/core/Module.h"
#include "zamt/core/ModuleCenter.h"
#include "zamt/core/Scheduler.h"

namespace zamt {

namespace dft_fftw_internal {
struct SubscriptionInfo {
  Scheduler::SourceId source_id;
  int subscription_id;
};
}  // namespace dft_fftw_internal

using namespace std::string_literals;

template <typename T>
class FourierTransform : public Module {
  std::string module_name;

 public:
  using Input = std::vector<T>;
  using Output = std::vector<std::complex<T>>;

  FourierTransform(int argc, const char* const* argv)
      : module_name("dft_fftw"s + "<"s + typeid(T).name() + ">"s),
        cli(argc, argv),
        logger(module_name.c_str(), cli) {
    logger.LogMessage("Starting...");
  }
  ~FourierTransform() {}

  void Initialize(const ModuleCenter* module_center) {
    logger.LogMessage(
        std::to_string(module_center->GetId<FourierTransform<T>>()).c_str());
  }

  int AddInputSource(Scheduler::SourceId /*source_id*/){};

 private:
  CLIParameters cli;
  Log logger;
};

}  // namespace zamt

#endif  // ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
