#pragma once

#include "FastLS/object/Box.hpp"
#include "FastLS/object/Container.hpp"
#include "FastLS/object/Cylinder.hpp"
#include "FastLS/object/Lidar.hpp"
#include "FastLS/sim/PIDController.hpp"
#include "FastLS/utils.hpp"

namespace fastls {
class Drone : public Container {
 public:
  Drone() { force_visible_ = true; }
  void Init() final {
    prop_[0]
        .SetSize(glm::vec3(0.3F, 0.3F, 0.1F))
        .SetPos(glm::vec3(-0.3F, -0.3F, -0.05F))
        .SetColor(utils::cyan)
        .SetLidarVisible(false);
    prop_[1]
        .SetSize(glm::vec3(0.3F, 0.3F, 0.1F))
        .SetPos(glm::vec3(0.3F, -0.3F, -0.05F))
        .SetColor(utils::cyan)
        .SetLidarVisible(false);
    prop_[2]
        .SetSize(glm::vec3(0.3F, 0.3F, 0.1F))
        .SetPos(glm::vec3(-0.3F, 0.3F, -0.05F))
        .SetColor(utils::cyan)
        .SetLidarVisible(false);
    prop_[3]
        .SetSize(glm::vec3(0.3F, 0.3F, 0.1F))
        .SetPos(glm::vec3(0.3F, 0.3F, -0.05F))
        .SetColor(utils::magenta)
        .SetLidarVisible(false);
    for (auto& p : prop_) {
      AddObject(&p);
    }

    lidar_.SetPos(glm::vec3(0.0F, 0.0F, 0.0F));
    AddObject(&lidar_);

    cyl_lidar_.SetSize(glm::vec3(0.2F, 0.2F, 0.2F))
        .SetPos(glm::vec3(0.0F, 0.0F, 0.1F))
        .SetColor(utils::black)
        .SetLidarVisible(false);
    AddObject(&cyl_lidar_);

    body_.SetSize(glm::vec3(0.5F, 0.5F, 0.1F))
        .SetColor(utils::dark_gray)
        .SetLidarVisible(false);
    AddObject(&body_);
  }

  fastls::Lidar lidar_;

 private:
  fastls::Cylinder prop_[4];
  fastls::Cylinder cyl_lidar_;
  fastls::Box body_;
  fastls::PIDController lidar_angle_pid_;
};

}  // namespace fastls