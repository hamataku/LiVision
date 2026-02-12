#pragma once

#include <memory>

#include "livision/Container.hpp"
#include "livision/ObjectBase.hpp"
#include "livision/marker/Arrow.hpp"

namespace livision {

/**
 * @brief XYZ axis arrows representing odometry pose.
 * @ingroup marker
 */
class Odometry : public Container, public SharedInstanceFactory<Odometry> {
 public:
  using Container::Container;
  /**
   * @brief Construct with initial parameters.
   */
  explicit Odometry(ObjectBase::Params params);

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
  std::shared_ptr<Arrow> arrow_x_;
  std::shared_ptr<Arrow> arrow_y_;
  std::shared_ptr<Arrow> arrow_z_;
};

}  // namespace livision
