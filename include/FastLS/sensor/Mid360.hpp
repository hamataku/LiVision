#pragma once

#include <memory>

#include "FastLS/Container.hpp"
#include "FastLS/ObjectBase.hpp"
#include "FastLS/obstacle/Cylinder.hpp"
#include "FastLS/sensor/LidarSensor.hpp"
#include "FastLS/sim/LidarSim.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class Mid360 : public Container {
 public:
  void InitImpl() final {
    body_.SetSize(glm::dvec3(0.06, 0.06, 0.03))
        .SetPos(glm::dvec3(0.0, 0.0, 0.015))
        .SetColor(utils::light_gray);
    AddObject(&body_);

    // mark_.SetSize(glm::dvec3(0.03, 0.03, 0.03))
    //     .SetPos(glm::dvec3(0.03, 0.0, 0.01))
    //     .SetColor(utils::red);
    // AddObject(&mark_);

    lidar_sensor_.SetPos(glm::dvec3(0, 0, 0.04));

    if (lidar_enable_) {
      lidar_sim.RegisterLidar(&lidar_sensor_);
    }
    AddObject(&lidar_sensor_);

    imu_place_.SetPos(glm::dvec3(0.011, 0.02329, -0.04412));
    AddObject(&imu_place_);
  }

  Mid360& SetLidarRange(float range) {
    lidar_sensor_.SetLidarRange(range);
    return *this;
  }

  Mid360& SetLidarEnable(bool enable) {
    lidar_enable_ = enable;
    return *this;
  }

  std::vector<glm::vec3>& GetPointClouds() {
    return lidar_sensor_.GetPointClouds();
  }

  ObjectBase imu_place_;

 private:
  Cylinder body_;
  // Cylinder mark_;
  LidarSensor lidar_sensor_;
  bool lidar_enable_ = true;
};

}  // namespace fastls
