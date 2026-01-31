#pragma once

#include <array>

#include "FastLS/Container.hpp"
#include "FastLS/obstacle/Box.hpp"
#include "FastLS/obstacle/Cylinder.hpp"
#include "FastLS/utils.hpp"

namespace fastls {
class DroneBody : public Container {
 public:
  void InitImpl() final {
    bool lidar_dynamic_observe = true;

    prop_.at(0)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(0.3, 0.3, 0.33))
        .SetColor(utils::magenta)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    prop_.at(1)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(-0.3, 0.3, 0.33))
        .SetColor(utils::cyan)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    prop_.at(2)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(-0.3, -0.3, 0.33))
        .SetColor(utils::cyan)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    prop_.at(3)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(0.3, -0.3, 0.33))
        .SetColor(utils::cyan)
        .SetLidarDynamicObserve(lidar_dynamic_observe);

    for (auto& p : prop_) {
      AddObject(&p);
    }
    legs_.at(0)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, -45.0))
        .SetPos(glm::dvec3(0.2, 0.2, 0.13))
        .SetColor(utils::white)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    legs_.at(1)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, 45.0))
        .SetPos(glm::dvec3(-0.2, 0.2, 0.13))
        .SetColor(utils::white)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    legs_.at(2)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, 135.0))
        .SetPos(glm::dvec3(-0.2, -0.2, 0.13))
        .SetColor(utils::white)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    legs_.at(3)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, 225.0))
        .SetPos(glm::dvec3(0.2, -0.2, 0.13))
        .SetColor(utils::white)
        .SetLidarDynamicObserve(lidar_dynamic_observe);

    for (auto& l : legs_) {
      AddObject(&l);
    }

    body_.SetSize(glm::dvec3(0.4, 0.4, 0.1))
        .SetPos(glm::dvec3(0, 0, 0.28))
        .SetColor(utils::dark_gray)
        .SetLidarDynamicObserve(lidar_dynamic_observe);
    AddObject(&body_);
  }

 private:
  std::array<Cylinder, 4> prop_;
  std::array<Cylinder, 4> legs_;
  Box body_;
};

}  // namespace fastls