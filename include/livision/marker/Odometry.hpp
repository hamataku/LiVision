#pragma once

#include "livision/Container.hpp"
#include "livision/marker/Arrow.hpp"

namespace livision {

/**
 * @brief XYZ axis arrows representing odometry pose.
 * @ingroup marker
 */
class Odometry : public Container {
 public:
  using Container::Container;
  /**
   * @brief Initialize axis arrows.
   */
  void OnInit() final;

  /**
   * @brief Set parameters for all axis arrows.
   */
  Odometry& SetArrowParams(const Arrow::ArrowParams& params);
  /**
   * @brief Set head length for all axis arrows.
   */
  Odometry& SetHeadLength(double length);
  /**
   * @brief Set head radius for all axis arrows.
   */
  Odometry& SetHeadRadius(double radius);
  /**
   * @brief Set body radius for all axis arrows.
   */
  Odometry& SetBodyRadius(double radius);

 private:
  Arrow arrow_x_;
  Arrow arrow_y_;
  Arrow arrow_z_;
};

}  // namespace livision