#pragma once

#include <memory>

#include "livision/Container.hpp"
#include "livision/object/primitives.hpp"

namespace livision {
/**
 * @defgroup object Object
 * @brief Renderable objects (Mesh, Drone, primitives).
 * @{
 */

/**
 * @brief Simple quadrotor model assembled from primitives.
 * @ingroup object
 */
class Drone : public Container, public SharedInstanceFactory<Drone> {
 public:
  /**
   * @brief Construct with initial parameters.
   */
  explicit Drone(ObjectBase::Params params = ObjectBase::Params())
      : Container(std::move(params)) {
    for (int i = 0; i < 4; i++) {
      float sign_x = (i < 2) ? 1.0F : -1.0F;
      float sign_y = (i % 2 == 0) ? 1.0F : -1.0F;

      auto prop =
          Cylinder::Instance({.pos = {0.3F * sign_x, 0.3F * sign_y, 0.33F},
                              .scale = {0.3F, 0.3F, 0.03F},
                              .color = color::cyan});
      AddObject(prop);

      auto leg =
          Cylinder::Instance({.pos = {0.2F * sign_x, 0.2F * sign_y, 0.13F},
                              .scale = {0.02F, 0.02F, 0.25F},
                              .color = color::white});
      leg->SetDegRotation({15.0F * sign_y, -15.0F * sign_x, 0.0F});
      AddObject(leg);
    }

    auto body = Box::Instance({.pos = {0.0, 0.0, 0.28},
                               .scale = {0.4, 0.4, 0.1},
                               .color = color::dark_gray});
    AddObject(body);
  }
};

/** @} */

}  // namespace livision
