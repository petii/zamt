#ifndef ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_
#define ZAMT_MODULE_FOURIER_TRANSFORM_FFTW_H_

#include <complex>
#include <vector>

#include "zamt/core/Scheduler.h"

namespace zamt {

template <typename T>
class FourierTransform {
 public:
  using input_t = std::vector<T>;
  using output_t = std::vector<std::complex<T>>;

  FourierTransform(Scheduler& scheduler, Scheduler::SourceId input_source_id,
                   Scheduler::SourceId output_source_id)
      : scheduler(scheduler) {
    // -Wunused variable
    output_source_id = input_source_id;
  }
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
