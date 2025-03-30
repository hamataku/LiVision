#pragma once

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class IMUObserver {
 public:
  explicit IMUObserver(ObjectBase* object, float dt)
      : object_(object), dt_(dt) {}

  void Update() {
    glm::mat4 mat = object_->GetGlobalMatrix();

    glm::quat cur_quat = glm::quat_cast(mat);
    auto cur_pos = glm::vec3(mat[3]);

    if (!init_) {
      init_ = true;
      prev_quat_ = cur_quat;
      prev_pos_ = cur_pos;
      return;
    }

    // Calculate linear acceleration in world frame
    glm::vec3 vel = (cur_pos - prev_pos_) / dt_;
    glm::vec3 world_acc = (vel - prev_vel_) / dt_;
    prev_vel_ = vel;

    // Add gravity in world frame
    world_acc += glm::vec3(0.0F, 0.0F, -gravity_);

    // Transform to local frame
    acc_ = glm::inverse(cur_quat) * world_acc;

    // Calculate angular velocity
    glm::quat diff_quat = cur_quat * glm::inverse(prev_quat_);
    glm::vec3 angular_vel_global = glm::eulerAngles(diff_quat) / dt_;
    angular_vel_ = glm::inverse(cur_quat) * angular_vel_global;

    prev_quat_ = cur_quat;
    prev_pos_ = cur_pos;
  }

  glm::vec3 GetAcc() const { return acc_; }
  glm::vec3 GetAngularVel() const { return angular_vel_; }

 private:
  const float gravity_ = 9.81F;
  bool init_ = false;

  glm::quat prev_quat_;
  glm::vec3 prev_pos_;
  glm::vec3 prev_vel_ = glm::vec3(0.0F);

  glm::vec3 acc_;
  glm::vec3 angular_vel_;

  ObjectBase* object_ = nullptr;
  float dt_ = 0.01F;  // Update interval
};
}  // namespace fastls
