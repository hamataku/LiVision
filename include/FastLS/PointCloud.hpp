#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/Box.hpp"
#include "FastLS/utils.hpp"
#include "object/ObjectBase.hpp"

namespace fastls {

class PointCloud : public ObjectBase {
 public:
  PointCloud() {
    box_.SetSize(glm::vec3(0.05F, 0.05F, 0.05F));
    color_ = utils::Color{1.0F, 0.0F, 0.0F, 1.0F};
  }
  void Draw(bgfx::ProgramHandle& program) final {
    box_.SetColor(color_);
    for (const auto& point : points_) {
      box_.SetPos(point);
      box_.Draw(program);
    }
  }

  void SetColor(utils::Color& color) { box_.SetColor(color); }
  void SetPoints(const std::vector<glm::vec3>& points) { points_ = points; }

 private:
  std::vector<glm::vec3> points_;
  Box box_;
};
}  // namespace fastls
