#pragma once

#include <random>

#include "FastLS/object/ObjectBase.hpp"
#include "common/utils.hpp"

namespace fastls {

class IMUObserver {
 public:
  /**
   * @brief IMUObserverを構築する
   * @param object 観測対象のオブジェクト
   * @param acc_psd 加速度計ノイズ密度 [m/s^2/√Hz]
   * @param gyr_psd ジャイロノイズ密度 [rad/s/√Hz]
   */
  explicit IMUObserver(ObjectBase* object, double dt, double acc_noise_density,
                       double gyr_noise_density,
                       glm::dvec3 acc_bias = glm::dvec3(0.0))
      : object_(object), dt_(dt), acc_bias_(acc_bias) {
    acc_dist_ =
        std::normal_distribution<>(0.0, acc_noise_density * std::sqrt(dt_));
    gyr_dist_ =
        std::normal_distribution<>(0.0, gyr_noise_density * std::sqrt(dt_));
  }

  void Update() {
    if (!object_) return;

    glm::mat4 mat_g = object_->GetGlobalMatrix();

    glm::dquat cur_quat_g = glm::quat_cast(mat_g);
    auto cur_pos_g = glm::dvec3(mat_g[3]);

    if (!init_) {
      init_ = true;
      prev_quat_g_ = cur_quat_g;
      prev_pos_g_ = cur_pos_g;
      acc_l_ = glm::inverse(cur_quat_g) * glm::dvec3(0.0, 0.0, kGravity);
      return;
    }

    // Calculate linear acceleration in world frame
    glm::dvec3 vel_g = (cur_pos_g - prev_pos_g_) / dt_;
    glm::dvec3 acc_g = (vel_g - prev_vel_g_) / dt_;
    prev_vel_g_ = vel_g;

    // Add gravity in world frame
    acc_g += glm::dvec3(0.0, 0.0, kGravity);

    // Transform to local frame
    acc_l_ = glm::inverse(cur_quat_g) * acc_g;
    acc_l_ +=
        glm::dvec3(acc_dist_(engine_), acc_dist_(engine_), acc_dist_(engine_)) +
        acc_bias_;
    // glm::dvec3 acc_l_new = glm::inverse(cur_quat_g) * acc_g;
    // glm::dvec3 acc_l_diff = acc_l_new - acc_l_;

    // if (glm::length(acc_l_diff) > kAccAbnormalityThreshold) {
    //   // 異常値検出したら、前の値を使う
    //   acc_l_diff = glm::dvec3(0.0);
    // }

    // acc_l_ += acc_l_diff + glm::dvec3(acc_dist_(engine_), acc_dist_(engine_),
    //                                   acc_dist_(engine_));

    // Calculate angular velocity
    glm::dquat diff_quat_g =
        glm::normalize(cur_quat_g * glm::inverse(prev_quat_g_));
    if (diff_quat_g.w < 0) diff_quat_g = -diff_quat_g;  // 符号の一貫性を保つ
    glm::dvec3 angular_vel_g;
    double sin_half_angle = std::sqrt(1.0 - (diff_quat_g.w * diff_quat_g.w));
    // 修正後
    if (sin_half_angle > 1e-6) {
      glm::dvec3 axis =
          glm::dvec3(diff_quat_g.x, diff_quat_g.y, diff_quat_g.z) /
          sin_half_angle;
      double angle = 2.0 * std::atan2(sin_half_angle, diff_quat_g.w);
      angular_vel_g = (angle / dt_) * axis;
    } else {
      // 小角度近似: angle ≈ 2 * sin(angle/2), axis*sin(angle/2) ≈ vec_part
      // angular_vel ≈ 2 * vec_part / dt
      angular_vel_g =
          (2.0 * glm::dvec3(diff_quat_g.x, diff_quat_g.y, diff_quat_g.z)) / dt_;
    }

    angular_vel_l_ = glm::inverse(cur_quat_g) * angular_vel_g;
    angular_vel_l_ +=
        glm::dvec3(gyr_dist_(engine_), gyr_dist_(engine_), gyr_dist_(engine_));

    prev_quat_g_ = cur_quat_g;
    prev_pos_g_ = cur_pos_g;
  }

  glm::dvec3 GetAcc() const { return acc_l_; }
  glm::dvec3 GetAngularVel() const { return angular_vel_l_; }

 private:
  static constexpr double kGravity = 9.81;
  static constexpr double kAccAbnormalityThreshold = 1.0;  // (m/s^2)

  bool init_ = false;

  glm::dquat prev_quat_g_;
  glm::dvec3 prev_pos_g_;
  glm::dvec3 prev_vel_g_ = glm::dvec3(0.0);

  glm::dvec3 acc_l_ = glm::dvec3(0.0);
  glm::dvec3 angular_vel_l_ = glm::dvec3(0.0);

  ObjectBase* object_ = nullptr;
  double dt_;

  std::mt19937 engine_{std::random_device{}()};
  std::normal_distribution<> acc_dist_;
  std::normal_distribution<> gyr_dist_;
  glm::dvec3 acc_bias_ = glm::dvec3(0.0);
};
}  // namespace fastls
