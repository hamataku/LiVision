#include "livision/marker/Grid.hpp"

namespace livision {

void Grid::OnDraw(Renderer& renderer) {
  cylinder_.SetColor(params_.color);

  // Draw grid lines along X axis
  for (double x = params_.pos.x() - (params_.scale.x() / 2);
       x <= params_.pos.x() + (params_.scale.x() / 2); x += resolution_) {
    cylinder_.SetPos(Eigen::Vector3d(x, params_.pos.y(), params_.pos.z()))
        ->SetScale(Eigen::Vector3d(0.05, 0.05, params_.scale.y()))
        ->SetDegRotation(Eigen::Vector3d(90.0, 0.0, 0.0));
    cylinder_.UpdateMatrix(global_mtx_);
    cylinder_.OnDraw(renderer);
  }
  // Draw grid lines along Y axis
  for (double y = params_.pos.y() - (params_.scale.y() / 2);
       y <= params_.pos.y() + (params_.scale.y() / 2); y += resolution_) {
    cylinder_.SetPos(Eigen::Vector3d(params_.pos.x(), y, params_.pos.z()))
        ->SetScale(Eigen::Vector3d(0.05, 0.05, params_.scale.x()))
        ->SetDegRotation(Eigen::Vector3d(0.0, 90.0, 0.0));
    cylinder_.UpdateMatrix(global_mtx_);
    cylinder_.OnDraw(renderer);
  }
}

void Grid::UpdateMatrix(const Eigen::Affine3d& parent_mtx) {
  if (local_mtx_changed_) {
    Eigen::Affine3d translation(Eigen::Translation3d(params_.pos));
    Eigen::Affine3d rotation(params_.quat);
    Eigen::Affine3d scale(Eigen::Scaling(params_.scale));

    local_mtx_ = translation * rotation * scale;
    local_mtx_changed_ = false;
  }
  // Keep legacy grid sizing semantics: params_.scale is treated as explicit
  // grid extent and should not be applied a second time via parent-local chain.
  global_mtx_ = parent_mtx;
}

Grid* Grid::SetResolution(double resolution) {
  resolution_ = resolution;
  return this;
}

}  // namespace livision
