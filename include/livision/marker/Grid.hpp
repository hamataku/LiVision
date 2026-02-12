#pragma once

#include "livision/object/primitives.hpp"

namespace livision {

/**
 * @brief Ground grid marker.
 * @ingroup marker
 */
class Grid : public ObjectBase, public SharedInstanceFactory<Grid> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Draw the grid.
   */
  void OnDraw(Renderer& renderer) override;
  /**
   * @brief Update cached transform matrices.
   */
  void UpdateMatrix(const Eigen::Affine3d& parent_mtx) override;
  /**
   * @brief Set grid resolution (line spacing).
   */
  Grid* SetResolution(double resolution);

 private:
  Cylinder cylinder_;
  double resolution_ = 1.0;
};
}  // namespace livision
