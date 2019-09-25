#include "fourierhandler.h"

#include <complex>
#include <iostream>

#include <fftw3.h>

namespace fh::detail {

struct fftwf_complex_allocator : std::allocator<fftwf_complex> {
  fftwf_complex* allocate(std::size_t size)
  {
    return reinterpret_cast<fftwf_complex*>(fftwf_malloc(sizeof(fftwf_complex) * size));
  }

  void deallocate(fftw_complex* ptr, std::size_t size) { fftw_free(ptr); }
};

struct HandlerImpl {
  unsigned size;

  FourierHandler::input_t input;
  std::vector<fftwf_complex, fftwf_complex_allocator> output;
  // FourierHandler::output_t output;

  fftwf_plan plan;

  HandlerImpl(unsigned);
  ~HandlerImpl();

  void addData(FourierHandler::input_t input);

  FourierHandler::output_t transform();
};

HandlerImpl::HandlerImpl(unsigned sampleSize)
    : size(sampleSize), input(size), output(size / 2 + 1),
      plan(fftwf_plan_dft_r2c_1d(size, input.data(),
                                 reinterpret_cast<fftwf_complex*>(output.data()),
                                 FFTW_MEASURE))
{
}

HandlerImpl::~HandlerImpl() { fftwf_destroy_plan(plan); }

void HandlerImpl::addData(FourierHandler::input_t data)
{
  // assert(data.size() == size);
  // input.assign(std::move_iterator(data.begin()), std::move_iterator(data.end()));
  input = data;
}

FourierHandler::output_t HandlerImpl::transform()
{
  fftwf_execute(plan);
  std::vector<std::complex<float>> tmp;
  tmp.reserve(size / 2 + 1);
  for (auto& item : output) {
    tmp.emplace_back(item[0], item[1]);
  }
  return tmp;
}

} // namespace fh::detail

FourierHandler::FourierHandler(unsigned sampleSize)
    : pimpl(std::make_unique<fh::detail::HandlerImpl>(sampleSize))
{
}

FourierHandler::~FourierHandler() = default;

FourierHandler::output_t FourierHandler::doTransform(input_t data)
{
  // std::clog << __FUNCTION__ << data.size() << std::endl;
  pimpl->addData(std::move(data));
  return std::move(pimpl->transform());
}