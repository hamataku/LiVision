#include "livision/marker/Arrow.hpp"

namespace livision {

void Arrow::OnDraw(Renderer& renderer) {
  // Calculate direction and length
  Eigen::Vector3d dir = arrow_params_.to_ - arrow_params_.from_;
  double length = dir.norm();
  const double eps = 1e-6;
  if (length < eps) {
    return;  // Avoid division by zero / degenerate arrow
  }
  Eigen::Vector3d dir_normalized = dir / length;

  // Clamp head length to not exceed total length
  double head_len = (arrow_params_.head_length_ < length)
                        ? arrow_params_.head_length_
                        : length;
  double body_len = length - head_len;

  // Calculate rotation quaternion from default Z axis to direction
  Eigen::Vector3d default_dir(0.0, 0.0, 1.0);
  Eigen::Quaterniond rotation =
      Eigen::Quaterniond::FromTwoVectors(default_dir, dir_normalized);
  rotation = rotation.normalized();

  // Draw body (only if it has positive length)
  if (body_len > eps) {
    Eigen::Vector3d body_center =
        arrow_params_.from_ + dir_normalized * (body_len * 0.5);
    body_.SetParams({
        .pos = body_center,
        .scale = Eigen::Vector3d(arrow_params_.body_radius_ * 2.0,
                                 arrow_params_.body_radius_ * 2.0, body_len),
        .quat = rotation,
        .color = params_.color,
        .wire_color = params_.wire_color,
    });
    body_.UpdateMatrix();
    body_.SetGlobalMatrix(global_mtx_ * body_.GetGlobalMatrix());
    body_.OnDraw(renderer);
  }

  // Draw head
  Eigen::Vector3d head_center =
      arrow_params_.from_ + dir_normalized * (body_len + (head_len * 0.5));
  head_.SetParams({
      .pos = head_center,
      .scale = Eigen::Vector3d(arrow_params_.head_radius_ * 2.0,
                               arrow_params_.head_radius_ * 2.0, head_len),
      .quat = rotation,
      .color = params_.color,
      .wire_color = params_.wire_color,
  });
  head_.UpdateMatrix();
  head_.SetGlobalMatrix(global_mtx_ * head_.GetGlobalMatrix());
  head_.OnDraw(renderer);
}

Arrow& Arrow::SetFromTo(const Eigen::Vector3d& from,
                        const Eigen::Vector3d& to) {
  arrow_params_.from_ = from;
  arrow_params_.to_ = to;
  return *this;
}

Arrow& Arrow::SetArrowParams(const ArrowParams& params) {
  arrow_params_ = params;
  return *this;
}
Arrow& Arrow::SetHeadLength(double length) {
  arrow_params_.head_length_ = length;
  return *this;
}
Arrow& Arrow::SetHeadRadius(double radius) {
  arrow_params_.head_radius_ = radius;
  return *this;
}
Arrow& Arrow::SetBodyRadius(double radius) {
  arrow_params_.body_radius_ = radius;
  return *this;
}

}  // namespace livision