#pragma once

#include "FastLS/object/Cylinder.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/object/Sphere.hpp"

namespace fastls {

class Path : public ObjectBase {
 public:
  void Draw(bgfx::ProgramHandle& program) final {
    cylinder_.SetColor(color_);
    sphere_.SetColor(color_);
    for (size_t i = 1; i < path_.size(); ++i) {
      glm::dvec3 p1 = path_[i - 1];
      glm::dvec3 p2 = path_[i];
      glm::dvec3 direction = glm::normalize(p2 - p1);
      float length = glm::distance(p1, p2);
      glm::dvec3 mid_point = (p1 + p2) / 2.0;
      cylinder_.SetPos(mid_point);
      cylinder_.SetSize(glm::dvec3(0.05, 0.05, length));
      cylinder_.SetQuatRotation(
          glm::quatLookAt(direction, glm::dvec3(0.0, 0.0, 1.0)));
      cylinder_.UpdateMatrix();
      cylinder_.Draw(program);

      if (is_sphere_) {
        sphere_.SetPos(p2).SetSize(glm::dvec3(0.1, 0.1, 0.1));
        sphere_.UpdateMatrix();
        sphere_.ForceSetGlobalMatrix(global_mtx_ * sphere_.GetGlobalMatrix());
        sphere_.Draw(program);
      }
    }
  }

  Path& SetPath(const std::vector<glm::vec3>& path) {
    path_ = path;
    return *this;
  }

  std::vector<glm::vec3>& GetPath() { return path_; }

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
