#pragma once

#include "FastLS/object/Container.hpp"
#include "FastLS/object/Cylinder.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class Odometry : public Container {
 public:
  void InitImpl() final {
    force_visible_ = true;  // Beore AddObject

    arrow_x_.SetSize(glm::dvec3(0.1, 0.1, 1.2))
        .SetPos(glm::dvec3(0.6, 0.0, 0.0))
        .SetDegRotation(glm::dvec3(0.0, 90.0, 0.0))
        .SetColor(utils::red)
        .SetLidarVisible(false);
    arrow_y_.SetSize(glm::dvec3(0.1, 0.1, 1.2))
        .SetPos(glm::dvec3(0.0, 0.6, 0.0))
        .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
        .SetColor(utils::green)
        .SetLidarVisible(false);
    arrow_z_.SetSize(glm::dvec3(0.1, 0.1, 1.2))
        .SetPos(glm::dvec3(0.0, 0.0, 0.6))
        .SetColor(utils::blue)
        .SetLidarVisible(false);
    AddObject(&arrow_x_);
    AddObject(&arrow_y_);
    AddObject(&arrow_z_);
  }

 private:
  fastls::Cylinder arrow_x_;
  fastls::Cylinder arrow_y_;
  fastls::Cylinder arrow_z_;
};

}  // namespace fastls