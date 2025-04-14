#pragma once

#include <random>

#include "FastLS/Settings.hpp"
#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class IMUObserver {
 public:
  explicit IMUObserver(ObjectBase* object, float acc_cov, float gyr_cov)
      : object_(object) {
    acc_dist_ = std::normal_distribution<>(
        0.0F, std::sqrt(acc_cov * settings::common_dt));
    gyr_dist_ = std::normal_distribution<>(
        0.0F, std::sqrt(gyr_cov * settings::common_dt));
  }

  void Update() {
    if (!object_) return;

    glm::mat4 mat_g = object_->GetGlobalMatrix();

    glm::quat cur_quat_g = glm::quat_cast(mat_g);
    auto cur_pos_g = glm::vec3(mat_g[3]);

    if (!init_) {
      init_ = true;
      prev_quat_g_ = cur_quat_g;
      prev_pos_g_ = cur_pos_g;
      acc_l_ = glm::inverse(cur_quat_g) * glm::vec3(0.0F, 0.0F, -gravity_);
      return;
    }

    // Calculate linear acceleration in world frame
    glm::vec3 vel_g = (cur_pos_g - prev_pos_g_) / settings::common_dt;
    glm::vec3 acc_g = (vel_g - prev_vel_g_) / settings::common_dt;
    prev_vel_g_ = vel_g;

    // Add gravity in world frame
    acc_g += glm::vec3(0.0F, 0.0F, -gravity_);

    // Transform to local frame
    acc_l_ = glm::inverse(cur_quat_g) * acc_g;
    acc_l_ +=
        glm::vec3(acc_dist_(engine_), acc_dist_(engine_), acc_dist_(engine_));

    // Calculate angular velocity
    glm::quat diff_quat_g =
        glm::normalize(cur_quat_g * glm::inverse(prev_quat_g_));
    if (diff_quat_g.w < 0) diff_quat_g = -diff_quat_g;  // 符号の一貫性を保つ
    glm::vec3 angular_vel_g;
    float sin_half_angle = std::sqrt(1.0F - (diff_quat_g.w * diff_quat_g.w));
    if (sin_half_angle > 1e-6F) {
      glm::vec3 axis = glm::vec3(diff_quat_g.x, diff_quat_g.y, diff_quat_g.z) /
                       sin_half_angle;
      angular_vel_g = (2.0F * std::atan2(sin_half_angle, diff_quat_g.w) /
                       settings::common_dt) *
                      axis;
    } else {
      angular_vel_g = glm::vec3(0.0F);
    }

    angular_vel_l_ = glm::inverse(cur_quat_g) * angular_vel_g;
    angular_vel_l_ +=
        glm::vec3(gyr_dist_(engine_), gyr_dist_(engine_), gyr_dist_(engine_));

    prev_quat_g_ = cur_quat_g;
    prev_pos_g_ = cur_pos_g;
  }

  glm::vec3 GetAcc() const { return acc_l_; }
  glm::vec3 GetAngularVel() const { return angular_vel_l_; }

 private:
  const float gravity_ = 9.81F;
  bool init_ = false;

  glm::quat prev_quat_g_;
  glm::vec3 prev_pos_g_;
  glm::vec3 prev_vel_g_ = glm::vec3(0.0F);

  glm::vec3 acc_l_ = glm::vec3(0.0F);
  glm::vec3 angular_vel_l_ = glm::vec3(0.0F);

  ObjectBase* object_ = nullptr;

  std::mt19937 engine_{std::random_device{}()};
  std::normal_distribution<> acc_dist_;
  std::normal_distribution<> gyr_dist_;
};
}  // namespace fastls
