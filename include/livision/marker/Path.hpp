#pragma once

#include "livision/object/primitives.hpp"

namespace livision {

class Path : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;

  void OnDraw(Renderer& renderer) final;

  Path& SetPath(const std::vector<Eigen::Vector3d>& path);
  std::vector<Eigen::Vector3d>& GetPath() { return path_; }

  Path& SetPathWidth(double width);
  Path& SetSphereVisible(bool is_sphere);
  Path& SetSphereSize(double size);

 private:
  Cylinder cylinder_;
  Sphere sphere_;

  std::vector<Eigen::Vector3d> path_;
  double width_ = 0.1;
  bool is_sphere_ = true;
  double sphere_size_ = 0.1;
};
}  // namespace livision
