#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/Box.hpp"
#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  PointCloud() {
    force_visible_ = true;
    color_ = utils::blue;
  }
  void Draw(bgfx::ProgramHandle& program) final {
    obj_.SetColor(color_);
    for (const auto& point : points_) {
      obj_.SetPos(point);
      obj_.SetSize(glm::vec3(point.w, point.w, point.w));
      obj_.UpdateMatrix();
      obj_.ForceSetGlobalMatrix(global_mtx_ * draw_mtx_ *
                                obj_.GetGlobalMatrix());
      obj_.Draw(program);
    }
  }

  PointCloud& SetVoxelSize(float size) {
    voxel_size_ = size;
    return *this;
  }

  void SetPoints(const std::vector<glm::vec3>& points) {
    points_.clear();
    for (const auto& p : points) {
      points_.emplace_back(p.x, p.y, p.z, voxel_size_);
    }
  }
  void SetPoints(std::vector<glm::vec3>& points, glm::mat4 mat) {
    SetPoints(points);
    draw_mtx_ = mat;
  }
  void SetPoints(const std::vector<glm::vec4>& points) { points_ = points; }

 private:
  std::vector<glm::vec4> points_;  // x,y,z,size
  float voxel_size_ = 0.12F;       // Default voxel size
  T obj_;
  glm::mat4 draw_mtx_ = glm::mat4(1.0F);
};
}  // namespace fastls
