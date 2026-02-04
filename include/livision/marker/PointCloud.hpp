#pragma once

#include "livision/Renderer.hpp"
#include "livision/object/primitives.hpp"

namespace livision {

template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  void OnDraw(Renderer& renderer) final {
    if (points_.empty()) return;

    auto& mesh_buf = obj_.GetMeshBuffer();

    if (mesh_buf)
      renderer.SubmitInstanced(*mesh_buf, points_, global_mtx_, params_.color);
  }

  PointCloud& SetPoints(const std::vector<Eigen::Vector3d>& points_with_size) {
    points_.clear();
    for (const auto& p : points_with_size) {
      points_.emplace_back(p.x(), p.y(), p.z(), size_);
    }
    return *this;
  }

  PointCloud& SetPoints(const std::vector<Eigen::Vector4d>& points) {
    points_ = points;
    return *this;
  }

  PointCloud& SetSize(double size) {
    size_ = size;
    return *this;
  }

  const std::vector<Eigen::Vector4d>& GetPoints() { return points_; }

 private:
  std::vector<Eigen::Vector4d> points_;
  T obj_;
  double size_ = 0.1;
};

}  // namespace livision
