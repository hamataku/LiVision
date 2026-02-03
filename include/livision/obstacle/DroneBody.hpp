#pragma once

#include <array>

#include "livision/Container.hpp"
#include "livision/obstacle/Box.hpp"
#include "livision/obstacle/Cylinder.hpp"
#include "livision/utils.hpp"

namespace livision {
class DroneBody : public Container {
 public:
  void InitImpl() final {
    prop_.at(0)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(0.3, 0.3, 0.33))
        .SetColorSpec(utils::magenta);
    prop_.at(1)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(-0.3, 0.3, 0.33))
        .SetColorSpec(utils::cyan);
    prop_.at(2)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(-0.3, -0.3, 0.33))
        .SetColorSpec(utils::cyan);
    prop_.at(3)
        .SetSize(glm::dvec3(0.3, 0.3, 0.03))
        .SetPos(glm::dvec3(0.3, -0.3, 0.33))
        .SetColorSpec(utils::cyan);

    for (auto& p : prop_) {
      AddObject(&p);
    }
    legs_.at(0)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, -45.0))
        .SetPos(glm::dvec3(0.2, 0.2, 0.13))
        .SetColorSpec(utils::white);
    legs_.at(1)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, 45.0))
        .SetPos(glm::dvec3(-0.2, 0.2, 0.13))
        .SetColorSpec(utils::white);
    legs_.at(2)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, 135.0))
        .SetPos(glm::dvec3(-0.2, -0.2, 0.13))
        .SetColorSpec(utils::white);
    legs_.at(3)
        .SetSize(glm::dvec3(0.01, 0.01, 0.25))
        .SetDegRotation(glm::dvec3(15.0, 0.0, 225.0))
        .SetPos(glm::dvec3(0.2, -0.2, 0.13))
        .SetColorSpec(utils::white);

    for (auto& l : legs_) {
      AddObject(&l);
    }

    body_.SetSize(glm::dvec3(0.4, 0.4, 0.1))
        .SetPos(glm::dvec3(0, 0, 0.28))
        .SetColorSpec(utils::dark_gray);
    AddObject(&body_);
  }

 private:
  std::array<Cylinder, 4> prop_;
  std::array<Cylinder, 4> legs_;
  Box body_;
};

}  // namespace livision