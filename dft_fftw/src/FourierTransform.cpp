#include "zamt/dft_fftw/FourierTransform.h"

#include <algorithm>
#include <chrono>
#include <complex>

#include <cassert>
#include <cstring>

#include <fftw3.h>

namespace zamt {
namespace dft_fftw {
namespace internal {

struct fftwf_complex_allocator : std::allocator<fftwf_complex> {
  fftwf_complex* allocate(std::size_t size) {
    return reinterpret_cast<fftwf_complex*>(
        fftwf_malloc(sizeof(fftwf_complex) * size));
  }

  void deallocate(fftw_complex* ptr, std::size_t /*size*/) { fftw_free(ptr); }
};

using input_t = std::vector<float>;
using output_t = std::vector<std::complex<float>>;

struct FFTW_Wrapper {
  unsigned size;

  input_t input;
  std::vector<fftwf_complex, fftwf_complex_allocator> output;

  fftwf_plan plan;

  FFTW_Wrapper(unsigned);
  ~FFTW_Wrapper();

  void addData(input_t input);
  void addPartialData(input_t input);
  output_t transform();
};

FFTW_Wrapper::FFTW_Wrapper(unsigned sampleSize)
    : size(sampleSize),
      input(size),
      output(size / 2 + 1),
      plan(fftwf_plan_dft_r2c_1d(static_cast<int>(size), input.data(),
                                 output.data(), FFTW_MEASURE)) {}

FFTW_Wrapper::~FFTW_Wrapper() { fftwf_destroy_plan(plan); }

void FFTW_Wrapper::addData(input_t data) {
  assert(data.size() == size);
  input = data;
}

void FFTW_Wrapper::addPartialData(input_t /*data*/) {
  // TODO: append to `input`
}

output_t FFTW_Wrapper::transform() {
  fftwf_execute(plan);
  output_t tmp;
  tmp.reserve(size / 2 + 1);
  for (auto& item : output) {
    tmp.emplace_back(item[0], item[1]);
  }
  return tmp;
}

}  // namespace internal

using clock = std::chrono::high_resolution_clock;

FourierTransform::FourierTransform(int argc, const char* const* argv)
    : module_name("dft_fftw"s), cli(argc, argv), log(module_name.c_str(), cli) {
  log.LogMessage("Starting...");
  if (cli.HasParam("-h")) {
    PrintHelp();
    return;
  }
  should_run_dft.store(true);
}

void FourierTransform::Initialize(const ModuleCenter* module_center) {
  if (!should_run_dft) return;

  log.Message("Initialize...");
  scheduler = &module_center->Get<Core>().scheduler();

#ifdef ZAMT_MODULE_LIVEAUDIO_PULSE
  auto audio = module_center->GetId<LiveAudio>();
  int subscriptionId = 0;
  log.Message("subscriptionId = ", subscriptionId);
  subscription = {audio, subscriptionId};

  auto packetSize = scheduler->GetPacketSize(audio);
  size_t sampleCount =
      static_cast<size_t>(packetSize) / sizeof(LiveAudio::StereoSample);
  log.Message("packetSize = ", packetSize, ", sampleCount = ", sampleCount);
  worker = std::make_unique<internal::FFTW_Wrapper>(sampleCount);

  auto self_id = module_center->GetId<FourierTransform>();

  worker->addData(internal::input_t(sampleCount));
  auto test_transform = worker->transform();
  // auto resultCount = sampleCount / 2 + 1;
  auto resultCount = test_transform.size();

  scheduler->Subscribe(
      audio,
      [=](auto id, auto packet, auto /*time*/) {
        static Scheduler::Time timestamp = 0;

        auto castedPacket =
            reinterpret_cast<const LiveAudio::StereoSample*>(packet);

        internal::input_t transformResult;
        transformResult.reserve(sampleCount);
        std::transform(castedPacket, castedPacket + sampleCount,
                       std::back_inserter(transformResult),
                       [](LiveAudio::StereoSample sample) {
                         return static_cast<float>(sample.left + sample.right) /
                                2.0f;
                       });

        scheduler->ReleasePacket(id, packet);

        worker->addData(transformResult);
        auto result = worker->transform();
        assert(result.size() == resultCount);

        auto resultPacket = reinterpret_cast<std::complex<float>*>(
            scheduler->GetPacketForSubmission(self_id));
        assert(resultPacket);

        memcpy(resultPacket, result.data(),
               result.size() * sizeof(std::complex<float>));

        scheduler->SubmitPacket(
            self_id, reinterpret_cast<Scheduler::Byte*>(resultPacket),
            timestamp++);
      },
      false, subscriptionId);
  log.Message("audio source = ", audio, ", self id = ", self_id);
  scheduler->RegisterSource(
      self_id, static_cast<int>(sizeof(std::complex<float>) * resultCount),
      42 /*random number, chosen by 2 fair dice rolls*/);
#endif

  // core.RegisterForQuitEvent([this](auto exit_code) {  });
}

}  // namespace dft_fftw
}  // namespace zamt
