#pragma once

#include "FastLS/object/Container.hpp"
#include "FastLS/object/Cylinder.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class Odometry : public Container {
 public:
  void InitImpl() final {
    force_visible_ = true;  // Before AddObject

    arrow_x_.SetSize(glm::vec3(0.1F, 0.1F, 1.2F))
        .SetPos(glm::vec3(0.6F, 0.0F, 0.0F))
        .SetDegRotation(glm::vec3(0.0F, 90.0F, 0.0F))
        .SetColor(utils::red)
        .SetLidarVisible(false);
    arrow_y_.SetSize(glm::vec3(0.1F, 0.1F, 1.2F))
        .SetPos(glm::vec3(0.0F, 0.6F, 0.0F))
        .SetDegRotation(glm::vec3(90.0F, 0.0F, 0.0F))
        .SetColor(utils::green)
        .SetLidarVisible(false);
    arrow_z_.SetSize(glm::vec3(0.1F, 0.1F, 1.2F))
        .SetPos(glm::vec3(0.0F, 0.0F, 0.6F))
        .SetColor(utils::blue)
        .SetLidarVisible(false);
    AddObject(&arrow_x_);
    AddObject(&arrow_y_);
    AddObject(&arrow_z_);
  }

 private:
  fastls::Cylinder arrow_x_;
  fastls::Cylinder arrow_y_;
  fastls::Cylinder arrow_z_;
};

}  // namespace fastls