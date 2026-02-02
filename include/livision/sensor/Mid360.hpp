#pragma once

#include "livision/Container.hpp"
#include "livision/ObjectBase.hpp"
#include "livision/obstacle/Cylinder.hpp"
#include "livision/sensor/LidarSensor.hpp"
#include "livision/sim/LidarSim.hpp"
#include "livision/utils.hpp"

namespace livision {

class Mid360 : public Container {
 public:
  void InitImpl() final {
    body_.SetSize(glm::dvec3(0.06, 0.06, 0.03))
        .SetPos(glm::dvec3(0.0, 0.0, 0.015))
        .SetColorSpec(utils::light_gray);
    AddObject(&body_);

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
  LidarSensor lidar_sensor_;
  bool lidar_enable_ = true;
};

}  // namespace livision
