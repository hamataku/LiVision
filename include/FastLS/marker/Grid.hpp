#pragma once

#include "LiVision/ObjectBase.hpp"
#include "LiVision/obstacle/Cylinder.hpp"

namespace livision {

class Grid : public ObjectBase {
 public:
  void Draw(bgfx::ProgramHandle& program) final {
    cylinder_.SetColorSpec(color_spec_);

    // Draw grid lines along X axis
    for (double x = pos_.x - (size_.x / 2); x <= pos_.x + (size_.x / 2);
         x += resolution_) {
      cylinder_.SetPos(glm::dvec3(x, pos_.y, pos_.z));
      cylinder_.SetSize(glm::dvec3(0.05, 0.05, size_.y));
      cylinder_.SetDegRotation(glm::dvec3(90.0, 0.0, 0.0));
      cylinder_.UpdateMatrix();
      cylinder_.Draw(program);
    }
    // Draw grid lines along Y axis
    for (double y = pos_.y - (size_.y / 2); y <= pos_.y + (size_.y / 2);
         y += resolution_) {
      cylinder_.SetPos(glm::dvec3(pos_.x, y, pos_.z));
      cylinder_.SetSize(glm::dvec3(0.05, 0.05, size_.x));
      cylinder_.SetDegRotation(glm::dvec3(0.0, 90.0, 0.0));
      cylinder_.UpdateMatrix();
      cylinder_.Draw(program);
    }
  }

  Grid& SetResolution(double resolution) {
    resolution_ = resolution;
    return *this;
  }

 private:
  Cylinder cylinder_;
  double resolution_ = 10.0;
};
}  // namespace livision
