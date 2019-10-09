#ifndef ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
#define ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_

#include <complex>
#include <vector>

#include "zamt/core/Module.h"
#include "zamt/core/ModuleCenter.h"
#include "zamt/core/Scheduler.h"

namespace zamt {

namespace detail {
template <typename T>
struct FourierTransformModule : public Module {
  FourierTransformModule(int, const char* const*) {}
  void Initialize(const ModuleCenter*) {}
};
}  // namespace detail

template <typename T>
class FourierTransform {
 public:
  using input_t = std::vector<T>;
  using output_t = std::vector<std::complex<T>>;

  FourierTransform(Scheduler& scheduler, Scheduler::SourceId input_source_id)
      : subscription_info{input_source_id, 0},
        output_source_id(static_cast<Scheduler::SourceId>(
            ModuleCenter::GetId<detail::FourierTransformModule<T>>())),
        scheduler(scheduler) {}

  Scheduler::SourceId getSourceId() const { return output_source_id; }

  ~FourierTransform() {}

 private:
  struct {
    Scheduler::SourceId source_id;
    int subscription_id;
  } subscription_info;

  Scheduler::SourceId output_source_id;

  Scheduler& scheduler;
};

}  // namespace zamt

#endif  // ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
