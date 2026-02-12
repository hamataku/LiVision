#pragma once

#include "livision/object/primitives.hpp"

namespace livision {

/**
 * @brief Polyline path marker with optional spheres.
 * @ingroup marker
 */
class Path : public ObjectBase, public SharedInstanceFactory<Path> {
 public:
  using ObjectBase::ObjectBase;

  /**
   * @brief Draw the path.
   */
  void OnDraw(Renderer& renderer) override;

  /**
   * @brief Set path points.
   */
  Path* SetPath(const std::vector<Eigen::Vector3d>& path);
  /**
   * @brief Get the current path points.
   */
  std::vector<Eigen::Vector3d>& GetPath() { return path_; }

  /**
   * @brief Set path width.
   */
  Path* SetPathWidth(double width);
  /**
   * @brief Toggle sphere markers on the path.
   */
  Path* SetSphereVisible(bool is_sphere);
  /**
   * @brief Set sphere marker size.
   */
  Path* SetSphereSize(double size);

 private:
  Cylinder cylinder_;
  Sphere sphere_;

  std::vector<Eigen::Vector3d> path_;
  double width_ = 0.1;
  bool is_sphere_ = false;
  double sphere_size_ = 0.1;
};
}  // namespace livision
