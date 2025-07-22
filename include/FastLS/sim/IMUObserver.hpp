#pragma once

#include <random>

#include "FastLS/Settings.hpp"
#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class IMUObserver {
 public:
  /**
   * @brief IMUObserverを構築する
   * @param object 観測対象のオブジェクト
   * @param acc_psd 加速度計ノイズのパワースペクトル密度 [(m/s^2)^2/Hz]
   * @param gyr_psd ジャイロノイズのパワースペクトル密度 [(rad/s)^2/Hz]
   */
  explicit IMUObserver(ObjectBase* object, double acc_cov, double gyr_cov)
      : object_(object) {
    acc_dist_ = std::normal_distribution<>(
        0.0F, std::sqrt(acc_cov * settings::common_dt));
    gyr_dist_ = std::normal_distribution<>(
        0.0F, std::sqrt(gyr_cov * settings::common_dt));
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
      acc_l_ = glm::inverse(cur_quat_g) * glm::dvec3(0.0, 0.0, gravity_);
      return;
    }

    // Calculate linear acceleration in world frame
    glm::dvec3 vel_g = (cur_pos_g - prev_pos_g_) / settings::common_dt;
    glm::dvec3 acc_g = (vel_g - prev_vel_g_) / settings::common_dt;
    prev_vel_g_ = vel_g;

    // Add gravity in world frame
    acc_g += glm::dvec3(0.0F, 0.0F, gravity_);

    // Transform to local frame
    acc_l_ = glm::inverse(cur_quat_g) * acc_g;
    acc_l_ +=
        glm::dvec3(acc_dist_(engine_), acc_dist_(engine_), acc_dist_(engine_));

    // Calculate angular velocity
    glm::dquat diff_quat_g =
        glm::normalize(cur_quat_g * glm::inverse(prev_quat_g_));
    if (diff_quat_g.w < 0) diff_quat_g = -diff_quat_g;  // 符号の一貫性を保つ
    glm::dvec3 angular_vel_g;
    double sin_half_angle = std::sqrt(1.0F - (diff_quat_g.w * diff_quat_g.w));
    // 修正後
    if (sin_half_angle > 1e-6F) {
      glm::dvec3 axis =
          glm::dvec3(diff_quat_g.x, diff_quat_g.y, diff_quat_g.z) /
          sin_half_angle;
      double angle = 2.0F * std::atan2(sin_half_angle, diff_quat_g.w);
      angular_vel_g = (angle / settings::common_dt) * axis;
    } else {
      // 小角度近似: angle ≈ 2 * sin(angle/2), axis*sin(angle/2) ≈ vec_part
      // angular_vel ≈ 2 * vec_part / dt
      angular_vel_g =
          (2.0 * glm::dvec3(diff_quat_g.x, diff_quat_g.y, diff_quat_g.z)) /
          settings::common_dt;
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
  const double gravity_ = 9.81F;
  bool init_ = false;

  glm::dquat prev_quat_g_;
  glm::dvec3 prev_pos_g_;
  glm::dvec3 prev_vel_g_ = glm::dvec3(0.0F);

  glm::dvec3 acc_l_ = glm::dvec3(0.0F);
  glm::dvec3 angular_vel_l_ = glm::dvec3(0.0F);

  ObjectBase* object_ = nullptr;

  std::mt19937 engine_{std::random_device{}()};
  std::normal_distribution<> acc_dist_;
  std::normal_distribution<> gyr_dist_;
};
}  // namespace fastls
