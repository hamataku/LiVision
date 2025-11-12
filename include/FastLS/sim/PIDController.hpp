#pragma once

namespace fastls {
struct PIDGains {
  double p = 1.0;
  double i = 0.0;
  double d = 0.0;
};

template <typename T>
class PIDController {
 public:
  explicit PIDController(double dt) : dt_(dt) { Reset(); }
  void Reset() {
    integral_ = T();
    prev_error_ = T();
  }

  void SetGains(const PIDGains& gains) { gains_ = gains; }

  T Update(const T& error) {
    integral_ += error * dt_;
    T derivative = (error - prev_error_) / dt_;
    prev_error_ = error;

    return (gains_.p * error) + (gains_.i * integral_) +
           (gains_.d * derivative);
  }

 private:
  PIDGains gains_;
  double dt_;
  T integral_ = T();
  T prev_error_ = T();
};

}  // namespace fastls
