#pragma once

#include "livision/ObjectBase.hpp"

namespace livision {

/**
 * @brief Box primitive.
 */
class Box : public ObjectBase {
 public:
  /**
   * @brief Construct a box with optional parameters.
   */
  explicit Box(Params params = Params());
};

/**
 * @brief Cone primitive.
 */
class Cone : public ObjectBase {
 public:
  /**
   * @brief Construct a cone with optional parameters.
   */
  explicit Cone(Params params = Params());
};

/**
 * @brief Cylinder primitive.
 */
class Cylinder : public ObjectBase {
 public:
  /**
   * @brief Construct a cylinder with optional parameters.
   */
  explicit Cylinder(Params params = Params());
};

/**
 * @brief Plane primitive.
 */
class Plane : public ObjectBase {
 public:
  /**
   * @brief Construct a plane with optional parameters.
   */
  explicit Plane(Params params = Params());
};

/**
 * @brief Sphere primitive.
 */
class Sphere : public ObjectBase {
 public:
  /**
   * @brief Construct a sphere with optional parameters.
   */
  explicit Sphere(Params params = Params());
};
}  // namespace livision
