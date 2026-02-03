#pragma once

#include <array>

#include "livision/Container.hpp"
#include "livision/object/primitives.hpp"

namespace livision {
class DroneBody : public Container {
 public:
  using Container::Container;

  void OnInit() final {
    prop_.at(0).SetParams({.pos = {0.3, 0.3, 0.33},
                           .scale = {0.3, 0.3, 0.03},
                           .color = color::cyan});
    prop_.at(1).SetParams({.pos = {-0.3, 0.3, 0.33},
                           .scale = {0.3, 0.3, 0.03},
                           .color = color::cyan});
    prop_.at(2).SetParams({.pos = {-0.3, -0.3, 0.33},
                           .scale = {0.3, 0.3, 0.03},
                           .color = color::cyan});
    prop_.at(3).SetParams({.pos = {0.3, -0.3, 0.33},
                           .scale = {0.3, 0.3, 0.03},
                           .color = color::cyan});
    for (auto& p : prop_) {
      AddObject(&p);
    }

    legs_.at(0)
        .SetParams({.pos = {0.2, 0.2, 0.13},
                    .scale = {0.01, 0.01, 0.25},
                    .color = color::white})
        .SetDegRotation({15.0, 0.0, -45.0});

    legs_.at(1)
        .SetParams({.pos = {-0.2, 0.2, 0.13},
                    .scale = {0.01, 0.01, 0.25},
                    .color = color::white})
        .SetDegRotation({15.0, 0.0, 45.0});
    legs_.at(2)
        .SetParams({.pos = {-0.2, -0.2, 0.13},
                    .scale = {0.01, 0.01, 0.25},
                    .color = color::white})
        .SetDegRotation({15.0, 0.0, 135.0});
    legs_.at(3)
        .SetParams({.pos = {0.2, -0.2, 0.13},
                    .scale = {0.01, 0.01, 0.25},
                    .color = color::white})
        .SetDegRotation({15.0, 0.0, 225.0});
    for (auto& l : legs_) {
      AddObject(&l);
    }

    body_.SetParams({.pos = {0.0, 0.0, 0.28},
                     .scale = {0.4, 0.4, 0.1},
                     .color = color::dark_gray});
    AddObject(&body_);
  }

 private:
  std::array<Cylinder, 4> prop_;
  std::array<Cylinder, 4> legs_;
  Box body_;
};

}  // namespace livision