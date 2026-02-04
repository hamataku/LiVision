#include "livision/marker/Odometry.hpp"

namespace livision {

void Odometry::OnInit() {
  arrow_x_
      .SetFromTo(Eigen::Vector3d(0.0, 0.0, 0.0), Eigen::Vector3d(1.2, 0.0, 0.0))
      .SetColor(color::red);
  arrow_y_
      .SetFromTo(Eigen::Vector3d(0.0, 0.0, 0.0), Eigen::Vector3d(0.0, 1.2, 0.0))
      .SetColor(color::green);
  arrow_z_
      .SetFromTo(Eigen::Vector3d(0.0, 0.0, 0.0), Eigen::Vector3d(0.0, 0.0, 1.2))
      .SetColor(color::blue);
  AddObject(&arrow_x_);
  AddObject(&arrow_y_);
  AddObject(&arrow_z_);
}

Odometry& Odometry::SetArrowParams(const Arrow::ArrowParams& params) {
  arrow_x_.SetArrowParams(params);
  arrow_y_.SetArrowParams(params);
  arrow_z_.SetArrowParams(params);
  return *this;
}
Odometry& Odometry::SetHeadLength(double length) {
  arrow_x_.SetHeadLength(length);
  arrow_y_.SetHeadLength(length);
  arrow_z_.SetHeadLength(length);
  return *this;
}
Odometry& Odometry::SetHeadRadius(double radius) {
  arrow_x_.SetHeadRadius(radius);
  arrow_y_.SetHeadRadius(radius);
  arrow_z_.SetHeadRadius(radius);
  return *this;
}
Odometry& Odometry::SetBodyRadius(double radius) {
  arrow_x_.SetBodyRadius(radius);
  arrow_y_.SetBodyRadius(radius);
  arrow_z_.SetBodyRadius(radius);
  return *this;
}

}  // namespace livision