#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastGL/object/Box.hpp"
#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {

class PointCloud : public ObjectBase {
 public:
  PointCloud() {
    box_.SetSize(glm::vec3(0.1F, 0.1F, 0.1F));
    box_.SetColor(color_);
  }
  void Draw(bgfx::ProgramHandle& program) final {
    for (const auto& point : points_) {
      box_.SetPos(point);
      box_.Draw(program);
    }
  }

  void SetColor(std::array<float, 4>& color) { box_.SetColor(color); }
  void SetPoints(const std::vector<glm::vec3>& points) { points_ = points; }

 private:
  std::vector<glm::vec3> points_;
  std::array<float, 4> color_{1.0F, 0.0F, 0.0F, 1.0F};  // RGBA
  Box box_;
};
}  // namespace fastgl
