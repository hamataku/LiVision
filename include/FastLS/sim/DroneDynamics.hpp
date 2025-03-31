#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class DroneDynamics {
 public:
  explicit DroneDynamics(ObjectBase *object, float dt)
      : object_(object), dt_(dt) {}

  void Init(glm::vec3 initial_pos, glm::vec3 initial_angle) {
    pos_ = initial_pos;
    angle_ = initial_angle;
    vel_ = glm::vec3(0.0F);
    vel_prev_ = glm::vec3(0.0F);
    angular_vel_ = glm::vec3(0.0F);
    angular_vel_prev_ = glm::vec3(0.0F);
    target_acc_ = glm::vec3(0.0F);
    target_anglular_acc_ = glm::vec3(0.0F);
    actual_acc_ = glm::vec3(0.0F);
  }

  void Update() {
    // Position
    vel_ += target_acc_ * dt_;
    if (glm::length(vel_) > 0.5F) {
      vel_ = glm::normalize(vel_) * 0.5F;
    }
    actual_acc_ = (vel_ - vel_prev_) / dt_;

    vel_prev_ = vel_;
    pos_ += vel_ * dt_;

    // Angle
    angular_vel_ += target_anglular_acc_ * dt_;
    if (glm::length(angular_vel_) > 0.05F) {
      angular_vel_ = glm::normalize(angular_vel_) * 0.05F;
    }
    angle_ += angular_vel_ * dt_;

    // Update object position and rotation
    object_->SetPos(pos_);
    object_->SetRadRotation(angle_);
  }

  void SetTargetAcc(const glm::vec3 &acc) { target_acc_ = acc; }
  void SetTargetAngularAcc(const glm::vec3 &angular_acc) {
    target_anglular_acc_ = angular_acc;
  }
  void SetMaxVel(float max_vel) { max_vel_ = max_vel; }
  void SetMaxAngularVel(float max_angular_vel) {
    max_anglular_vel_ = max_angular_vel;
  }

  glm::vec3 GetPos() const { return pos_; }
  glm::vec3 GetAngle() const { return angle_; }

 private:
  ObjectBase *object_ = nullptr;
  // Settings
  float dt_;
  float max_vel_ = 1.0F;
  float max_anglular_vel_ = 0.1F;

  // Position
  glm::vec3 target_acc_ = glm::vec3(0.0F);

  glm::vec3 pos_ = glm::vec3(0.0F);
  glm::vec3 vel_ = glm::vec3(0.0F);
  glm::vec3 vel_prev_ = glm::vec3(0.0F);
  glm::vec3 actual_acc_ = glm::vec3(0.0F);

  // Angle
  glm::vec3 target_anglular_acc_ = glm::vec3(0.0F);

  glm::vec3 angle_ = glm::vec3(0.0F);
  glm::vec3 angular_vel_ = glm::vec3(0.0F);
  glm::vec3 angular_vel_prev_ = glm::vec3(0.0F);
};

}  // namespace fastls