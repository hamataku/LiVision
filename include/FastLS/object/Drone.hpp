#pragma once

#include "FastLS/object/Box.hpp"
#include "FastLS/object/Lidar.hpp"

namespace fastls {
class Drone : public Container {
 public:
  void InitImpl() final {
    bool lidar_dynamic_observe = true;

    prop_[0]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(-0.3, -0.3, -0.11))
        .SetColor(utils::cyan)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    prop_[1]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(0.3, -0.3, -0.11))
        .SetColor(utils::cyan)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    prop_[2]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(-0.3, 0.3, -0.11))
        .SetColor(utils::cyan)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    prop_[3]
        .SetSize(glm::dvec3(0.3, 0.3, 0.1))
        .SetPos(glm::dvec3(0.3, 0.3, -0.11))
        .SetColor(utils::magenta)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    for (auto& p : prop_) {
      AddObject(&p);
    }

    AddObject(&lidar_);

    body_.SetSize(glm::dvec3(0.5, 0.5, 0.1))
        .SetPos(glm::dvec3(0, 0, -0.1))
        .SetColor(utils::dark_gray)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    AddObject(&body_);
  }

  Lidar lidar_;

 private:
  Cylinder prop_[4];
  Box body_;
};

}  // namespace fastls