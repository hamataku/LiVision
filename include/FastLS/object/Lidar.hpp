#pragma once

#include "FastLS/object/Container.hpp"
#include "FastLS/object/Cylinder.hpp"
#include "FastLS/sensor/LidarSensor.hpp"
#include "FastLS/sim/LidarSim.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class Lidar : public Container {
 public:
  void InitImpl() final {
    body_.SetSize(glm::dvec3(0.06, 0.06, 0.03))
        .SetPos(glm::dvec3(0.0, 0.0, 0.015))
        .SetColor(utils::light_gray);
    AddObject(&body_);

    lidar_sensor_.SetPos(glm::dvec3(0, 0, 0.04));

    if (lidar_enable_) {
      lidar_sim.RegisterLidar(&lidar_sensor_);
    }
    AddObject(&lidar_sensor_);
  }

  Lidar& SetLidarRange(float range) {
    lidar_sensor_.SetLidarRange(range);
    return *this;
  }

  Lidar& SetLidarEnable(bool enable) {
    lidar_enable_ = enable;
    return *this;
  }

  std::vector<glm::vec3>& GetPointClouds() {
    return lidar_sensor_.GetPointClouds();
  }

 private:
  Cylinder body_;
  LidarSensor lidar_sensor_;
  bool lidar_enable_ = true;
};
}  // namespace fastls
