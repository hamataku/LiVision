#pragma once

#include "livision/Renderer.hpp"
#include "livision/object/primitives.hpp"

namespace livision {

/**
 * @brief Visualizes degeneracy directions for translation and rotation.
 * @ingroup marker
 */
class DegeneracyIndicator : public ObjectBase,
                            public SharedInstanceFactory<DegeneracyIndicator> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Initialize indicator primitives.
   */
  void OnInit() final;
  /**
   * @brief Draw the indicator.
   */
  void OnDraw(Renderer& renderer) final;

  /**
   * @brief Set degeneracy vectors for translation and rotation.
   */
  DegeneracyIndicator& SetDegeneracyInfo(
      const std::vector<Eigen::Vector3d>& degen_trans,
      const std::vector<Eigen::Vector3d>& degen_rot);

 private:
  std::vector<Eigen::Vector3d> degen_trans_;
  std::vector<Eigen::Vector3d> degen_rot_;

  std::array<Cylinder, 3> trans_cyl_;
  Cylinder rot_cyl_;
  Sphere rot_sphere_;
};
}  // namespace livision
