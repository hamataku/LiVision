#pragma once

#include "FastLS/object/DroneBody.hpp"
#include "FastLS/object/Lidar.hpp"

namespace fastls {
class DroneLidarUp : public Container {
 public:
  void InitImpl() final {
    lidar_.SetPos(glm::dvec3(0, 0, 0.4)).SetRadRotation(lidar_angle_offset_);
    AddObject(&lidar_);

    AddObject(&body_);
  }

  Lidar lidar_;
  glm::dvec3 lidar_angle_offset_{0, 0, 0};

 private:
  DroneBody body_;
};

}  // namespace fastls