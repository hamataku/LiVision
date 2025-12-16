#pragma once

#include "FastLS/object/DroneBody.hpp"
#include "FastLS/object/Mid360.hpp"

namespace fastls {
class DroneLidarDown : public Container {
 public:
  void InitImpl() final {
    lidar_.SetPos(glm::dvec3(0, 0, 0.2)).SetRadRotation(lidar_angle_offset_);
    AddObject(&lidar_);

    AddObject(&body_);
  }

  Mid360 lidar_;
  glm::dvec3 lidar_angle_offset_{0, M_PI, 0};

 private:
  DroneBody body_;
};

}  // namespace fastls