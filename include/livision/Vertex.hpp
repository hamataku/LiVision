#pragma once

namespace livision {
/**
 * @brief Simple 3D vertex with arithmetic operators.
 */
struct Vertex {
  float x;
  float y;
  float z;
  float u = 0.0F;
  float v = 0.0F;

  /**
   * @brief Equality comparison.
   */
  bool operator==(const Vertex& other) const {
    return x == other.x && y == other.y && z == other.z && u == other.u &&
           v == other.v;
  }

  /**
   * @brief Inequality comparison.
   */
  bool operator!=(const Vertex& other) const { return !(*this == other); }

  /**
   * @brief
   *
   */
  Vertex operator+(const Vertex& other) const {
    return Vertex{.x = x + other.x, .y = y + other.y, .z = z + other.z, .u = u,
                  .v = v};
  }

  /**
   * @brief In-place vector addition.
   */
  Vertex& operator+=(const Vertex& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  /**
   * @brief Vector subtraction.
   */
  Vertex operator-(const Vertex& other) const {
    return Vertex{.x = x - other.x, .y = y - other.y, .z = z - other.z, .u = u,
                  .v = v};
  }

  /**
   * @brief In-place vector subtraction.
   */
  Vertex& operator-=(const Vertex& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  /**
   * @brief Unary negation.
   */
  Vertex operator-() const {
    return Vertex{.x = -x, .y = -y, .z = -z, .u = u, .v = v};
  }

  /**
   * @brief Scalar multiplication.
   */
  Vertex operator*(float s) const {
    return Vertex{.x = x * s, .y = y * s, .z = z * s, .u = u, .v = v};
  }

  /**
   * @brief In-place scalar multiplication.
   */
  Vertex& operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  /**
   * @brief Scalar division.
   */
  Vertex operator/(float s) const {
    return Vertex{.x = x / s, .y = y / s, .z = z / s, .u = u, .v = v};
  }

  /**
   * @brief In-place scalar division.
   */
  Vertex& operator/=(float s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }
};

/**
 * @brief Scalar multiplication (commutative form).
 */
inline Vertex operator*(float s, const Vertex& v) {
  return Vertex{
      .x = v.x * s, .y = v.y * s, .z = v.z * s, .u = v.u, .v = v.v};
}
}  // namespace livision
