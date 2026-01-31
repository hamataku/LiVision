#pragma once

#include "FastLS/object/obstacle/DroneBody.hpp"
#include "FastLS/object/sensor/Mid360.hpp"

namespace fastls {
class DroneLidarUp : public Container {
 public:
  void InitImpl() final {
    lidar_.SetPos(glm::dvec3(0, 0, 0.4)).SetRadRotation(lidar_angle_offset_);
    AddObject(&lidar_);

    AddObject(&body_);
  }

  Mid360 lidar_;
  glm::dvec3 lidar_angle_offset_{0, 0, 0};

 private:
  DroneBody body_;
};

}  // namespace fastls