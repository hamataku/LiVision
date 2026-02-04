#pragma once

#include "livision/Renderer.hpp"
#include "livision/object/primitives.hpp"

namespace livision {

class DegeneracyIndicator : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;
  void OnInit() final;
  void OnDraw(Renderer& renderer) final;

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
