#pragma once

#include "../LidarSim.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Lidar : public ObjectBase {
 public:
  void Init() final { lidar_sim.RegisterLidar(this); }
};
}  // namespace fastls
