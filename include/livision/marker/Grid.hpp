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
  void OnDraw(Renderer& renderer) final;

  /**
   * @brief Set grid resolution (line spacing).
   */
  Grid& SetResolution(double resolution);

 private:
  Cylinder cylinder_;
  double resolution_ = 10.0;
};
}  // namespace livision
