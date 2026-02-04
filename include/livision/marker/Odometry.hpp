#pragma once

#include "livision/Container.hpp"
#include "livision/marker/Arrow.hpp"

namespace livision {

class Odometry : public Container {
 public:
  using Container::Container;
  void OnInit() final;

  Odometry& SetArrowParams(const Arrow::ArrowParams& params);
  Odometry& SetHeadLength(double length);
  Odometry& SetHeadRadius(double radius);
  Odometry& SetBodyRadius(double radius);

 private:
  Arrow arrow_x_;
  Arrow arrow_y_;
  Arrow arrow_z_;
};

}  // namespace livision