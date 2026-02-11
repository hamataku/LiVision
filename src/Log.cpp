#include "livision/Log.hpp"

#include <atomic>
#include <iostream>

namespace livision {
namespace {
std::atomic<int> g_log_level{static_cast<int>(LogLevel::Off)};
}

void SetLogLevel(LogLevel level) {
  g_log_level.store(static_cast<int>(level), std::memory_order_relaxed);
}

LogLevel GetLogLevel() {
  return static_cast<LogLevel>(g_log_level.load(std::memory_order_relaxed));
}

bool ShouldLog(LogLevel level) {
  if (level == LogLevel::Off) {
    return false;
  }
  return static_cast<int>(level) <=
         g_log_level.load(std::memory_order_relaxed);
}

void Log(LogLevel level, const std::string& message) {
  if (!ShouldLog(level)) {
    return;
  }
  if (level <= LogLevel::Warn) {
    std::cerr << "[LiVision] " << message << std::endl;
  } else {
    std::cout << "[LiVision] " << message << std::endl;
  }
}

}  // namespace livision
