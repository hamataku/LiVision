#pragma once

#include "FastLS/object/DroneBody.hpp"
#include "FastLS/object/Lidar.hpp"

namespace fastls {
class DroneLidarUp : public Container {
 public:
  void InitImpl() final {
    lidar_.SetPos(glm::dvec3(0, 0, 0.25));
    AddObject(&lidar_);

    AddObject(&body_);
  }

  Lidar lidar_;

 private:
  DroneBody body_;
};

}  // namespace fastls