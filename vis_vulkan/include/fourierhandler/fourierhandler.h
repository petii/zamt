#pragma once

#include <vector>
#include <complex>
#include <memory>

namespace fh::detail {

struct HandlerImpl;

}

class FourierHandler {

public:
  using input_t = std::vector<float>;
  using output_t = std::vector<std::complex<float>>;

  FourierHandler(unsigned sampleSize);
  ~FourierHandler();

  output_t doTransform(input_t input);

private:
  std::unique_ptr<fh::detail::HandlerImpl> pimpl;
};