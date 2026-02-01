#pragma once

#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/ObjectBase.hpp"
#include "FastLS/obstacle/Box.hpp"

namespace fastls {

template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  void Draw(bgfx::ProgramHandle& program) final {
    for (const auto& point : points_) {
      obj_.SetColorSpec(color_spec_);
      obj_.SetPos(glm::dvec3(point.x, point.y, point.z));
      obj_.SetSize(glm::dvec3(point.w, point.w, point.w));
      obj_.UpdateMatrix();
      obj_.ForceSetGlobalMatrix(global_mtx_ * obj_.GetGlobalMatrix());
      obj_.Draw(program);
    }
  }

  PointCloud& SetVoxelSize(float size) {
    voxel_size_ = size;
    return *this;
  }

  PointCloud& SetPoints(const std::vector<glm::vec3>& points) {
    points_.clear();
    for (const auto& p : points) {
      points_.emplace_back(p.x, p.y, p.z, voxel_size_);
    }
    return *this;
  }

  PointCloud& SetPoints(const std::vector<glm::vec4>& points) {
    points_ = points;
    return *this;
  }

  const std::vector<glm::vec4>& GetPoints() { return points_; }

 private:
  std::vector<glm::vec4> points_;
  double voxel_size_ = 0.12;
  T obj_;
};

}  // namespace fastls
