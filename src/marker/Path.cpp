#include "livision/marker/Path.hpp"

namespace livision {

void Path::OnDraw(Renderer& renderer) {
  cylinder_.SetColor(params_.color);
  sphere_.SetColor(params_.color);
  for (size_t i = 1; i < path_.size(); ++i) {
    Eigen::Vector3d p1 = path_[i - 1];
    Eigen::Vector3d p2 = path_[i];
    Eigen::Vector3d direction = (p2 - p1).normalized();
    float length = (p2 - p1).norm();
    Eigen::Vector3d mid_point = (p1 + p2) / 2.0;
    cylinder_.SetPos(mid_point);
    cylinder_.SetScale(Eigen::Vector3d(width_, width_, length));
    cylinder_.SetQuatRotation(Eigen::Quaterniond::FromTwoVectors(
        Eigen::Vector3d(0, 0, 1), direction));
    cylinder_.UpdateMatrix();
    cylinder_.OnDraw(renderer);

    if (is_sphere_) {
      sphere_.SetPos(p2)->SetScale(
          Eigen::Vector3d(sphere_size_, sphere_size_, sphere_size_));
      sphere_.UpdateMatrix();
      sphere_.OnDraw(renderer);
    }
  }
}

Path& Path::SetPath(const std::vector<Eigen::Vector3d>& path) {
  path_ = path;
  return *this;
}
Path& Path::SetPathWidth(double width) {
  width_ = width;
  return *this;
}

Path& Path::SetSphereVisible(bool is_sphere) {
  is_sphere_ = is_sphere;
  return *this;
}
Path& Path::SetSphereSize(double size) {
  sphere_size_ = size;
  return *this;
}

}  // namespace livision
