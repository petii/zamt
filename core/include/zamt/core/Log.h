#ifndef ZAMT_CORE_LOG_H_
#define ZAMT_CORE_LOG_H_

#include "zamt/core/CLIParameters.h"

#include <sstream>
#include <string>
/// Very simple handling of output to console.

namespace zamt {

namespace internal {
template <typename Arg>
void stringify(std::ostream& stream, Arg&& arg) {
  stream << std::forward<Arg>(arg);
}

template <typename Arg, typename... Args>
void stringify(std::ostream& stream, Arg&& arg, Args&&... args) {
  stream << std::forward<Arg>(arg);
  stringify(stream, std::forward<Args>(args)...);
}
}  // namespace internal

class Log {
 public:
  const static char* kVerboseParamStr;

  /// Label is prefixed to every log message. Set verbose mode.
  Log(const char* label, const CLIParameters& cli);

  /// Unconditionally print message to console.
  static void Print(const char* messa);
  /// Print help for verbose handling.
  static void PrintHelp4Verbose();

  /// Log only if verbose mode is on, output message in nice log format.
  void LogMessage(const char* msg);
  void LogMessage(const char* msg, int num, const char* suffix = "");
  void LogMessage(const char* msg, float num, const char* suffix = "");

  template <typename... Args>
  void Message(Args&&... args) {
    std::ostringstream stream;
    internal::stringify(stream, std::forward<Args>(args)...);
    LogMessage(stream.str().c_str());
  }

 private:
  const char* label_;
  bool verbose_;
};

}  // namespace zamt

#endif  // ZAMT_CORE_LOG_H_
