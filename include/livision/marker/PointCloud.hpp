#pragma once

#include "livision/Renderer.hpp"
#include "livision/object/primitives.hpp"

namespace livision {

/**
 * @brief Point cloud marker rendered as instanced primitives.
 * @tparam T Primitive type used for each point.
 */
template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  /**
   * @brief Draw the point cloud.
   */
  void OnDraw(Renderer& renderer) final {
    if (points_.empty()) return;

    auto& mesh_buf = obj_.GetMeshBuffer();

    if (mesh_buf)
      renderer.SubmitInstanced(*mesh_buf, points_, global_mtx_, params_.color);
  }

  /**
   * @brief Set points from 3D positions (uniform size).
   */
  PointCloud& SetPoints(const std::vector<Eigen::Vector3d>& points_with_size) {
    points_.clear();
    for (const auto& p : points_with_size) {
      points_.emplace_back(p.x(), p.y(), p.z(), size_);
    }
    return *this;
  }

  /**
   * @brief Set points with per-point size in w component.
   */
  PointCloud& SetPoints(const std::vector<Eigen::Vector4d>& points) {
    points_ = points;
    return *this;
  }

  /**
   * @brief Set the uniform point size.
   */
  PointCloud& SetSize(double size) {
    size_ = size;
    return *this;
  }

  /**
   * @brief Get current points.
   */
  const std::vector<Eigen::Vector4d>& GetPoints() { return points_; }

 private:
  std::vector<Eigen::Vector4d> points_;
  T obj_;
  double size_ = 0.1;
};

}  // namespace livision
