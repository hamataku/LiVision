#pragma once
#include <Eigen/Dense>

#include "FastLS/Settings.hpp"

namespace fastls {

class PIDController {
 public:
  struct PIDGains {
    double p = 1.0;
    double i = 0.0;
    double d = 0.0;
  };

  void Reset() {
    integral_ = 0.0;
    prev_error_ = 0.0;
    integral_vec_ = Eigen::Vector3d::Zero();
    prev_error_vec_ = Eigen::Vector3d::Zero();
  }

  void SetGains(const PIDGains& gains) { gains_ = gains; }

  // スカラー値用のPID制御
  double Update(double error) {
    integral_ += error * settings::common_dt;
    double derivative = (error - prev_error_) / settings::common_dt;
    prev_error_ = error;

    return (gains_.p * error) + (gains_.i * integral_) +
           (gains_.d * derivative);
  }

  // ベクトル値用のPID制御
  Eigen::Vector3d Update(const Eigen::Vector3d& error) {
    integral_vec_ += error * settings::common_dt;
    Eigen::Vector3d derivative =
        (error - prev_error_vec_) / settings::common_dt;
    prev_error_vec_ = error;

    return (gains_.p * error) + (gains_.i * integral_vec_) +
           (gains_.d * derivative);
  }

 private:
  PIDGains gains_;

  // スカラー値用の状態
  double integral_ = 0.0;
  double prev_error_ = 0.0;

  // ベクトル値用の状態
  Eigen::Vector3d integral_vec_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d prev_error_vec_ = Eigen::Vector3d::Zero();
};

}  // namespace fastls
