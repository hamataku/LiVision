#pragma once

namespace livision {
/**
 * @brief Simple 3D vertex with arithmetic operators.
 */
struct Vertex {
  float x;
  float y;
  float z;

  /**
   * @brief Equality comparison.
   */
  bool operator==(const Vertex& other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  /**
   * @brief Inequality comparison.
   */
  bool operator!=(const Vertex& other) const { return !(*this == other); }

  /**
   * @brief Vector addition.
   */
  Vertex operator+(const Vertex& other) const {
    return Vertex{x + other.x, y + other.y, z + other.z};
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
    return Vertex{x - other.x, y - other.y, z - other.z};
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
  Vertex operator-() const { return Vertex{-x, -y, -z}; }

  /**
   * @brief Scalar multiplication.
   */
  Vertex operator*(float s) const { return Vertex{x * s, y * s, z * s}; }

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
  Vertex operator/(float s) const { return Vertex{x / s, y / s, z / s}; }

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
  return Vertex{v.x * s, v.y * s, v.z * s};
}
}  // namespace livision