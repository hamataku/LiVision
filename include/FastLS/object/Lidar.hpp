#pragma once

#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Lidar : public ObjectBase {
 public:
  void Init() final { lidar_sim.RegisterLidar(this); }
};
}  // namespace fastls
