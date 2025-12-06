#pragma once

#include "FastLS/object/DroneBody.hpp"
#include "FastLS/object/Lidar.hpp"

namespace fastls {
class DroneLidarDown : public Container {
 public:
  void InitImpl() final {
    lidar_.SetPos(glm::dvec3(0, 0, 0.2)).SetDegRotation(glm::dvec3(-180, 0, 0));
    AddObject(&lidar_);

    AddObject(&body_);
  }

  Lidar lidar_;

 private:
  DroneBody body_;
};

}  // namespace fastls