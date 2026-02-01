#pragma once

#include "LiVision/obstacle/DroneBody.hpp"
#include "LiVision/sensor/Mid360.hpp"

namespace livision {
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

}  // namespace livision