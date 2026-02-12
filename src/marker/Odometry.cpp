#include "livision/marker/Odometry.hpp"

namespace livision {

Odometry::Odometry(ObjectBase::Params params) : Container(std::move(params)) {
  arrow_x_ = Arrow::Instance();
  arrow_y_ = Arrow::Instance();
  arrow_z_ = Arrow::Instance();

  arrow_x_
      ->SetFromTo(Eigen::Vector3d(0.0, 0.0, 0.0),
                  Eigen::Vector3d(1.2, 0.0, 0.0))
      .SetColor(color::red);
  arrow_y_
      ->SetFromTo(Eigen::Vector3d(0.0, 0.0, 0.0),
                  Eigen::Vector3d(0.0, 1.2, 0.0))
      .SetColor(color::green);
  arrow_z_
      ->SetFromTo(Eigen::Vector3d(0.0, 0.0, 0.0),
                  Eigen::Vector3d(0.0, 0.0, 1.2))
      .SetColor(color::blue);
  AddObject(arrow_x_);
  AddObject(arrow_y_);
  AddObject(arrow_z_);
}

Odometry& Odometry::SetArrowParams(const Arrow::ArrowParams& params) {
  if (arrow_x_) {
    arrow_x_->SetArrowParams(params);
  }
  if (arrow_y_) {
    arrow_y_->SetArrowParams(params);
  }
  if (arrow_z_) {
    arrow_z_->SetArrowParams(params);
  }
  return *this;
}
Odometry& Odometry::SetHeadLength(double length) {
  if (arrow_x_) {
    arrow_x_->SetHeadLength(length);
  }
  if (arrow_y_) {
    arrow_y_->SetHeadLength(length);
  }
  if (arrow_z_) {
    arrow_z_->SetHeadLength(length);
  }
  return *this;
}
Odometry& Odometry::SetHeadRadius(double radius) {
  if (arrow_x_) {
    arrow_x_->SetHeadRadius(radius);
  }
  if (arrow_y_) {
    arrow_y_->SetHeadRadius(radius);
  }
  if (arrow_z_) {
    arrow_z_->SetHeadRadius(radius);
  }
  return *this;
}
Odometry& Odometry::SetBodyRadius(double radius) {
  if (arrow_x_) {
    arrow_x_->SetBodyRadius(radius);
  }
  if (arrow_y_) {
    arrow_y_->SetBodyRadius(radius);
  }
  if (arrow_z_) {
    arrow_z_->SetBodyRadius(radius);
  }
  return *this;
}

}  // namespace livision
