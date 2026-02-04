#pragma once

#include "livision/object/primitives.hpp"

namespace livision {

class Grid : public ObjectBase {
 public:
  using ObjectBase::ObjectBase;
  void OnDraw(Renderer& renderer) final;

  Grid& SetResolution(double resolution);

 private:
  Cylinder cylinder_;
  double resolution_ = 10.0;
};
}  // namespace livision
