#pragma once

#include "FastLS/object/Box.hpp"
#include "FastLS/object/Lidar.hpp"

namespace fastls {
class Drone : public Container {
 public:
  Drone() { force_visible_ = true; }
  void InitImpl() final {
    prop_[0]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(-0.3, -0.3, -0.05))
        .SetColor(utils::cyan)
        .SetLidarVisible(false);
    prop_[1]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(0.3, -0.3, -0.05))
        .SetColor(utils::cyan)
        .SetLidarVisible(false);
    prop_[2]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(-0.3, 0.3, -0.05))
        .SetColor(utils::cyan)
        .SetLidarVisible(false);
    prop_[3]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(0.3, 0.3, -0.05))
        .SetColor(utils::magenta)
        .SetLidarVisible(false);
    for (auto& p : prop_) {
      AddObject(&p);
    }

    AddObject(&lidar_);

    body_.SetSize(glm::dvec3(0.5, 0.5, 0.1))
        .SetColor(utils::dark_gray)
        .SetLidarVisible(false);
    AddObject(&body_);
  }

  Lidar lidar_;

 private:
  Cylinder prop_[4];
  Box body_;
};

}  // namespace fastls