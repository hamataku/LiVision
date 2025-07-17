#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/Box.hpp"
#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class PointCloud : public ObjectBase {
 public:
  PointCloud() {
    box_.SetSize(glm::vec3(0.12F, 0.12F, 0.12F));
    force_visible_ = true;
    color_ = utils::blue;
  }
  void Draw(bgfx::ProgramHandle& program) final {
    box_.SetColor(color_);
    for (const auto& point : points_) {
      box_.SetPos(point);
      box_.UpdateMatrix();
      box_.ForceSetGlobalMatrix(global_mtx_ * draw_mtx_ *
                                box_.GetGlobalMatrix());
      box_.Draw(program);
    }
  }

  PointCloud& SetVoxelSize(float size) {
    box_.SetSize(glm::vec3(size, size, size));
    return *this;
  }

  void SetPoints(const std::vector<glm::vec3>& points) { points_ = points; }
  void SetPoints(std::vector<glm::vec3>& points, glm::mat4 mat) {
    points_ = points;
    draw_mtx_ = mat;
  }

 private:
  std::vector<glm::vec3> points_;
  Box box_;
  glm::mat4 draw_mtx_ = glm::mat4(1.0F);
};
}  // namespace fastls
