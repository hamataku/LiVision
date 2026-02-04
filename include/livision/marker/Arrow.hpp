#pragma once

#include <Eigen/Geometry>

#include "livision/object/primitives.hpp"

namespace livision {

class Arrow : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;

  struct ArrowParams {
    Eigen::Vector3d from_ = Eigen::Vector3d(0.0, 0.0, 0.0);
    Eigen::Vector3d to_ = Eigen::Vector3d(0.0, 0.0, 0.0);
    double head_length_ = 0.3;
    double head_radius_ = 0.06;
    double body_radius_ = 0.03;
  };

  void OnDraw(Renderer& renderer) override;

  Arrow& SetFromTo(const Eigen::Vector3d& from, const Eigen::Vector3d& to);

  Arrow& SetArrowParams(const ArrowParams& params);
  Arrow& SetHeadLength(double length);
  Arrow& SetHeadRadius(double radius);
  Arrow& SetBodyRadius(double radius);

 private:
  livision::Cylinder body_;
  livision::Cone head_;

  ArrowParams arrow_params_;
};

}  // namespace livision