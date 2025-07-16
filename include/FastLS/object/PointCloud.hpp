#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/Box.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class PointCloud : public ObjectBase {
 public:
  PointCloud() {
    box_.SetSize(glm::vec3(0.12F, 0.12F, 0.12F));
    force_visible_ = true;
  }
  void Draw(bgfx::ProgramHandle& program) final {
    for (const auto& point : points_) {
      box_.SetColor(CalcColor(point));
      box_.SetPos(point);
      box_.UpdateMatrix();
      box_.ForceSetGlobalMatrix(global_mtx_ * draw_mtx_ *
                                box_.GetGlobalMatrix());
      box_.Draw(program);
    }
  }

  void SetPoints(const std::vector<glm::vec3>& points) { points_ = points; }
  void SetPoints(std::vector<glm::vec3>& points, glm::mat4 mat) {
    points_ = points;
    draw_mtx_ = mat;
  }

 private:
  static utils::Color CalcColor(glm::vec3 point) {
    float z = point.z;
    float z_min = -2.0F;
    float z_max = 15.0F;
    float z_norm =
        std::max(0.0F, std::min(1.0F, (z - z_min) / (z_max - z_min)));

    // 高さに応じてRGB値を変化させる
    float r = std::min(1.0F, 2.0F * z_norm);  // 下半分で0→1に変化
    float g = std::min(1.0F, 2.0F * (z_norm - 0.5F));  // 中間で0→1に変化
    float b = 1.0F - r;  // 上半分で1→0に変化

    return utils::Color{r, g, b, 1.0F};
  }
  std::vector<glm::vec3> points_;
  Box box_;
  glm::mat4 draw_mtx_ = glm::mat4(1.0F);
};
}  // namespace fastls
