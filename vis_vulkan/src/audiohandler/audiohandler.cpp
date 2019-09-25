#include "audiohandler.h"

#include <atomic>
#include <iostream>
#include <iterator>
#include <numeric>
#include <thread>

#include <portaudiocpp/PortAudioCpp.hxx>

namespace pa = portaudio;

namespace ah {

constexpr auto framesPerBuffer = 512;
constexpr auto sampleRate = 44100;
constexpr auto channels = 2;
} // namespace ah

namespace ah::detail {

struct HandlerImpl {
  using stream_t = pa::MemFunCallbackStream<HandlerImpl>;

  pa::AutoSystem autoSystem;
  pa::System& system;

  pa::DirectionSpecificStreamParameters inputParameters;
  pa::DirectionSpecificStreamParameters outputParameters;
  pa::StreamParameters streamParameters;

  std::unique_ptr<stream_t> stream;

  int callback(const void*, void*, unsigned long, const PaStreamCallbackTimeInfo*,
               PaStreamCallbackFlags);

  using action_t = AudioHandler::action_t;
  using sound_t = AudioHandler::sample_t;

  action_t action;

  bool monitor;

  HandlerImpl(int, double, unsigned long, action_t, bool monitor = false);
};

HandlerImpl::HandlerImpl(int channels, double sampleRate, unsigned long framesPerBuffer,
                         action_t action, bool monitor)
    : system(pa::System::instance()),
      inputParameters(system.defaultInputDevice(), channels, pa::FLOAT32, false,
                      system.defaultInputDevice().defaultLowInputLatency(), nullptr),
      outputParameters(system.defaultOutputDevice(), channels, pa::FLOAT32, false,
                       system.defaultOutputDevice().defaultHighOutputLatency(), nullptr),
      streamParameters(inputParameters,
                       monitor ? outputParameters
                               : pa::DirectionSpecificStreamParameters::null(),
                       sampleRate, framesPerBuffer, paClipOff),
      stream(std::make_unique<stream_t>(streamParameters, *this, &HandlerImpl::callback)),
      action(action), monitor(monitor)
{
  std::clog << "Opened input stream with latency: " << stream->inputLatency() << " seconds" << std::endl;
}

int HandlerImpl::callback(const void* input, void* output, unsigned long frames,
                          const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags)
{
  sound_t data(frames);
  auto fInput = static_cast<const float* const*>(input);
  auto fOutput = static_cast<float**>(output);
  for (std::size_t i = 0; i < frames; ++i) {
    data[i] = (fInput[0][i] + fInput[1][i]) / 2;
    if (monitor) {
      fOutput[0][i] = data[i];
      fOutput[1][i] = data[i];
    }
  }
  action(std::move(data));
  data.clear();
  return paContinue;
}
} // namespace ah::detail

AudioHandler::AudioHandler(unsigned limit, double overlap, action_t callback,
                           bool monitor)
    : pimpl(std::make_unique<ah::detail::HandlerImpl>(
          ah::channels, ah::sampleRate, ah::framesPerBuffer,
          [this](auto data) { this->callback(std::move(data)); }, monitor)),
      action(callback), overlap(overlap), bufferLimit(limit),
      buffer(bufferLimit + bufferLimit * overlap, 0.0f)
{
}

AudioHandler::~AudioHandler() = default;

void AudioHandler::startRecording()
{
  std::clog << __FUNCTION__ << std::endl;
  pimpl->stream->start();
}

void AudioHandler::stopRecording()
{
  std::clog << __FUNCTION__ << std::endl;
  pimpl->stream->stop();
}

void AudioHandler::callback(sample_t data)
{
  static std::atomic<size_t> newDataCounter = 0;

  newDataCounter += data.size();
  buffer.insert(buffer.end(), std::move_iterator(data.begin()),
                std::move_iterator(data.end()));
  if (newDataCounter >= bufferLimit) {
    newDataCounter = bufferLimit * overlap;
    sample_t tmp(buffer.begin(), buffer.begin() + bufferLimit);
    std::thread a{[this, data = std::move(tmp)]() { action(std::move(data)); }};
    a.detach();
  }
}