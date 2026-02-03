#pragma once

#include "livision/ObjectBase.hpp"

namespace livision {

class Box : public ObjectBase {
 public:
  explicit Box(Params params = Params());
};

class Cone : public ObjectBase {
 public:
  explicit Cone(Params params = Params());
};

class Cylinder : public ObjectBase {
 public:
  explicit Cylinder(Params params = Params());
};

class Plane : public ObjectBase {
 public:
  explicit Plane(Params params = Params());
};

class Sphere : public ObjectBase {
 public:
  explicit Sphere(Params params = Params());
};
}  // namespace livision
