#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/Box.hpp"
#include "FastLS/utils.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class PointCloud : public ObjectBase {
 public:
  PointCloud() {
    box_.SetSize(glm::vec3(0.05F, 0.05F, 0.05F));
    color_ = utils::red.Alpha(0.5F);
    force_visible_ = true;
  }
  void Draw(bgfx::ProgramHandle& program) final {
    for (const auto& point : points_) {
      box_.SetColor(color_);
      box_.SetPos(point);
      box_.UpdateMatrix();
      box_.ForceSetGlobalMatrix(draw_mtx_ * box_.GetGlobalMatrix());
      box_.Draw(program);
    }
  }

  void SetPoints(const std::vector<glm::vec3>& points) {
    points_ = std::move(points);
  }
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
