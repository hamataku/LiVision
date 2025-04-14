#pragma once
#include <Eigen/Dense>

#include "FastLS/Settings.hpp"

namespace fastls {
struct PIDGains {
  double p = 1.0;
  double i = 0.0;
  double d = 0.0;
};

template <typename T>
class PIDController {
 public:
  PIDController() { Reset(); }
  void Reset() {
    integral_ = T();
    prev_error_ = T();
  }

  void SetGains(const PIDGains& gains) { gains_ = gains; }

  T Update(const T& error) {
    integral_ += error * settings::common_dt;
    T derivative = (error - prev_error_) / settings::common_dt;
    prev_error_ = error;

    return (gains_.p * error) + (gains_.i * integral_) +
           (gains_.d * derivative);
  }

 private:
  PIDGains gains_;
  T integral_ = T();
  T prev_error_ = T();
};

}  // namespace fastls
