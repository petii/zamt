#pragma once

#include <memory>
#include <functional>
#include <vector>

#include <boost/circular_buffer.hpp>

namespace ah::detail {
  struct HandlerImpl;
}

class AudioHandler {
  std::unique_ptr<ah::detail::HandlerImpl> pimpl;

public:

  using sample_t = std::vector<float>;
  using action_t = std::function<void(sample_t)>;


  AudioHandler(unsigned, double, action_t, bool monitor);
  virtual ~AudioHandler();

  void startRecording();
  void stopRecording();

private:
  void callback(sample_t);

  action_t action;

  double overlap;
  unsigned bufferLimit;
  boost::circular_buffer<float> buffer;
};