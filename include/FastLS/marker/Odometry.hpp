#pragma once

#include "FastLS/Container.hpp"
#include "FastLS/marker/Arrow.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class Odometry : public Container {
 public:
  void InitImpl() final {
    arrow_x_.SetFromTo(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(1.2, 0.0, 0.0))
        .SetColor(utils::red);
    arrow_y_.SetFromTo(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 1.2, 0.0))
        .SetColor(utils::green);
    arrow_z_.SetFromTo(glm::dvec3(0.0, 0.0, 0.0), glm::dvec3(0.0, 0.0, 1.2))
        .SetColor(utils::blue);
    AddObject(&arrow_x_);
    AddObject(&arrow_y_);
    AddObject(&arrow_z_);
  }

 private:
  Arrow arrow_x_;
  Arrow arrow_y_;
  Arrow arrow_z_;
};

}  // namespace fastls