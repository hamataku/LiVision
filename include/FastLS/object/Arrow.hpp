#pragma once

#include <glm/gtx/quaternion.hpp>

#include "FastLS/object/Cone.hpp"
#include "FastLS/object/Cylinder.hpp"

namespace fastls {

class Arrow : public ObjectBase {
 public:
  void Draw(bgfx::ProgramHandle& program) override {
    // Calculate direction and length
    glm::dvec3 dir = to_ - from_;
    double length = glm::length(dir);
    const double eps = 1e-6;
    if (length < eps) {
      return;  // Avoid division by zero / degenerate arrow
    }
    glm::dvec3 dir_normalized = dir / length;

    // Clamp head length to not exceed total length
    double head_len = (head_length_ < length) ? head_length_ : length;
    double body_len = length - head_len;

    // Calculate rotation quaternion from default Z axis to direction
    glm::dvec3 default_dir = glm::dvec3(0.0, 0.0, 1.0);
    glm::dquat rotation = glm::rotation(default_dir, dir_normalized);
    rotation = glm::normalize(rotation);

    // Draw body (only if it has positive length)
    if (body_len > eps) {
      glm::dvec3 body_center = from_ + dir_normalized * (body_len * 0.5);
      body_.SetPos(body_center)
          .SetQuatRotation(rotation)
          .SetSize(glm::dvec3(body_radius_ * 2.0, body_radius_ * 2.0, body_len))
          .SetColor(color_);
      body_.UpdateMatrix();
      body_.ForceSetGlobalMatrix(global_mtx_ * body_.GetGlobalMatrix());
      body_.Draw(program);
    }

    // Draw head
    glm::dvec3 head_center =
        from_ + dir_normalized * (body_len + (head_len * 0.5));
    head_.SetPos(head_center)
        .SetQuatRotation(rotation)
        .SetSize(glm::dvec3(head_radius_ * 2.0, head_radius_ * 2.0, head_len))
        .SetColor(color_);
    head_.UpdateMatrix();
    head_.ForceSetGlobalMatrix(global_mtx_ * head_.GetGlobalMatrix());
    head_.Draw(program);
  }

  Arrow& SetFromTo(const glm::dvec3& from, const glm::dvec3& to) {
    from_ = from;
    to_ = to;
    return *this;
  }

  Arrow& SetHeadLength(double length) {
    head_length_ = length;
    return *this;
  }
  Arrow& SetHeadRadius(double radius) {
    head_radius_ = radius;
    return *this;
  }
  Arrow& SetBodyRadius(double radius) {
    body_radius_ = radius;
    return *this;
  }

 private:
  fastls::Cylinder body_;
  fastls::Cone head_;

  glm::dvec3 from_ = glm::dvec3(0.0, 0.0, 0.0);
  glm::dvec3 to_ = glm::dvec3(0.0, 0.0, 1.0);

  double head_length_ = 0.3;
  double head_radius_ = 0.06;
  double body_radius_ = 0.03;
};

}  // namespace fastls