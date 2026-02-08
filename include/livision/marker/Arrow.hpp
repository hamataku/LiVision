#pragma once

#include <Eigen/Geometry>

#include "livision/object/primitives.hpp"

namespace livision {

/**
 * @defgroup marker Marker
 * @brief Marker objects (Arrow, Grid, Path, etc.).
 * @{
 */

/**
 * @brief Arrow marker composed of a cylinder body and cone head.
 * @ingroup marker
 */
class Arrow : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;

  /**
   * @brief Parameters describing arrow geometry.
   */
  struct ArrowParams {
    Eigen::Vector3d from_ = Eigen::Vector3d(0.0, 0.0, 0.0);
    Eigen::Vector3d to_ = Eigen::Vector3d(0.0, 0.0, 0.0);
    double head_length_ = 0.3;
    double head_radius_ = 0.06;
    double body_radius_ = 0.03;
  };

  /**
   * @brief Draw the arrow.
   */
  void OnDraw(Renderer& renderer) override;

  /**
   * @brief Set the arrow start and end points.
   */
  Arrow& SetFromTo(const Eigen::Vector3d& from, const Eigen::Vector3d& to);

  /**
   * @brief Set all arrow parameters.
   */
  Arrow& SetArrowParams(const ArrowParams& params);
  /**
   * @brief Set head length.
   */
  Arrow& SetHeadLength(double length);
  /**
   * @brief Set head radius.
   */
  Arrow& SetHeadRadius(double radius);
  /**
   * @brief Set body radius.
   */
  Arrow& SetBodyRadius(double radius);

 private:
  livision::Cylinder body_;
  livision::Cone head_;

  ArrowParams arrow_params_;
};

/** @} */

}  // namespace livision