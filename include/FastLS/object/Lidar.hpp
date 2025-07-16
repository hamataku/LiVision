#pragma once

#include "FastLS/object/Container.hpp"
#include "FastLS/object/Cylinder.hpp"
#include "FastLS/object/LidarSensor.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Lidar : public Container {
 public:
  void InitImpl() final {
    body_.SetSize(glm::vec3(0.2F, 0.2F, 0.5F))
        .SetPos(glm::vec3(0.0F, 0.0F, 0.25F))
        .SetColor(utils::black)
        .SetLidarVisible(false);
    AddObject(&body_);

    lidar_sim.RegisterLidar(&lidar_sensor_);
    AddObject(&lidar_sensor_);
  }

  ObjectBase& SetLidarRange(float range) {
    lidar_sensor_.SetLidarRange(range);
    return *this;
  }

  std::vector<glm::vec3>& GetPointClouds() {
    return lidar_sensor_.GetPointClouds();
  }

 private:
  Cylinder body_;
  LidarSensor lidar_sensor_;
};
}  // namespace fastls
