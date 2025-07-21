#pragma once

#include <cstddef>

#include "FastLS/object/Cylinder.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/object/Sphere.hpp"

namespace fastls {

class Path : public ObjectBase {
 public:
  Path() { force_visible_ = true; }
  void Draw(bgfx::ProgramHandle& program) final {
    cylinder_.SetColor(color_);
    sphere_.SetColor(color_);
    for (size_t i = 1; i < path_.size(); ++i) {
      glm::vec3 p1 = path_[i - 1];
      glm::vec3 p2 = path_[i];
      glm::vec3 direction = glm::normalize(p2 - p1);
      float length = glm::distance(p1, p2);
      glm::vec3 mid_point = (p1 + p2) / 2.0F;
      cylinder_.SetPos(mid_point);
      cylinder_.SetSize(glm::vec3(0.1F, 0.1F, length));
      cylinder_.SetQuatRotation(
          glm::quatLookAt(direction, glm::vec3(0.0F, 0.0F, 1.0F)));
      cylinder_.UpdateMatrix();
      cylinder_.Draw(program);

      if (is_sphere_) {
        sphere_.SetPos(p2).SetSize(glm::vec3(0.2F, 0.2F, 0.2F));
        sphere_.UpdateMatrix();
        sphere_.Draw(program);
      }
    }
  }

  Path& SetPath(const std::vector<glm::vec3>& path) {
    path_ = path;
    return *this;
  }

  Path& SetSphereVisible(bool is_sphere) {
    is_sphere_ = is_sphere;
    return *this;
  }

 private:
  Cylinder cylinder_;
  Sphere sphere_;
  std::vector<glm::vec3> path_;
  bool is_sphere_ = true;  // Draw spheres at the end of the path
};
}  // namespace fastls
