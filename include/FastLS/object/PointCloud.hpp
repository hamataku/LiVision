#pragma once

#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/object/Box.hpp"
#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  PointCloud() { color_ = utils::blue; }

  void Draw(bgfx::ProgramHandle& program) final {
    obj_.SetColor(color_);
    for (const auto& point : points_) {
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
  // PointCloud& SetPoints(std::vector<glm::vec3>& points, glm::mat4 draw_mtx) {
  //   SetPoints(points);
  //   draw_mtx_ = draw_mtx;
  //   return *this;
  // }
  PointCloud& SetPoints(const std::vector<glm::vec4>& points) {
    points_ = points;
    return *this;
  }
  const std::vector<glm::vec4>& GetPoints() { return points_; }
  // glm::dmat4 GetDrawMatrix() const { return draw_mtx_; }

 private:
  std::vector<glm::vec4> points_;  // x,y,z,size
  double voxel_size_ = 0.12;       // Default voxel size
  T obj_;
  // glm::dmat4 draw_mtx_ = glm::dmat4(1.0);
};
}  // namespace fastls
