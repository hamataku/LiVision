#pragma once

#include <glm/fwd.hpp>

#include "FastLS/object/Box.hpp"
#include "FastLS/object/Container.hpp"
#include "FastLS/object/Lidar.hpp"
#include "FastLS/utils.hpp"

namespace fastls {
class Drone : public Container {
 public:
  Drone() { force_visible_ = true; }
  void InitImpl() final {
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

    AddObject(&lidar_);

    body_.SetSize(glm::vec3(0.5F, 0.5F, 0.1F))
        .SetColor(utils::dark_gray)
        .SetLidarVisible(false);
    AddObject(&body_);
  }

  Lidar lidar_;

 private:
  Cylinder prop_[4];
  Box body_;
};

}  // namespace fastls