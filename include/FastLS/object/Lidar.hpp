#pragma once

#include "FastLS/object/Container.hpp"
#include "FastLS/object/Cylinder.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Lidar : public Container {
 public:
  void Init() final {
    lidar_sim.RegisterLidar(this);

    body_.SetSize(glm::vec3(0.2F, 0.2F, 0.5F))
        .SetPos(glm::vec3(0.0F, 0.0F, 0.25F))
        .SetColor(utils::black)
        .SetLidarVisible(false);
    AddObject(&body_);
  }

 private:
  Cylinder body_;
};
}  // namespace fastls
