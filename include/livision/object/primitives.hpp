#pragma once

#include "livision/ObjectBase.hpp"

namespace livision {

/**
 * @brief Box primitive.
 * @ingroup object
 */
class Box : public ObjectBase, public SharedInstanceFactory<Box> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Construct a box with optional parameters.
   */
  explicit Box(Params params = Params());
};

/**
 * @brief Cone primitive.
 * @ingroup object
 */
class Cone : public ObjectBase, public SharedInstanceFactory<Cone> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Construct a cone with optional parameters.
   */
  explicit Cone(Params params = Params());
};

/**
 * @brief Cylinder primitive.
 * @ingroup object
 */
class Cylinder : public ObjectBase, public SharedInstanceFactory<Cylinder> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Construct a cylinder with optional parameters.
   */
  explicit Cylinder(Params params = Params());
};

/**
 * @brief Plane primitive.
 * @ingroup object
 */
class Plane : public ObjectBase, public SharedInstanceFactory<Plane> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Construct a plane with optional parameters.
   */
  explicit Plane(Params params = Params());
};

/**
 * @brief Sphere primitive.
 * @ingroup object
 */
class Sphere : public ObjectBase, public SharedInstanceFactory<Sphere> {
 public:
  using ObjectBase::ObjectBase;
  /**
   * @brief Construct a sphere with optional parameters.
   */
  explicit Sphere(Params params = Params());
};
}  // namespace livision
