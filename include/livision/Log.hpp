#pragma once

#include <sstream>
#include <string>
#include <utility>

namespace livision {

enum class LogLevel { Off = 0, Error = 1, Warn = 2, Info = 3, Debug = 4 };

void SetLogLevel(LogLevel level);
LogLevel GetLogLevel();
bool ShouldLog(LogLevel level);
void Log(LogLevel level, const std::string& message);

template <class... Args>
void LogMessage(LogLevel level, Args&&... args) {
  if (!ShouldLog(level)) {
    return;
  }
  std::ostringstream oss;
  (oss << ... << std::forward<Args>(args));
  Log(level, oss.str());
}

}  // namespace livision
