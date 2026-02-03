#pragma once

namespace livision {
struct Vertex {
  float x;
  float y;
  float z;

  bool operator==(const Vertex& other) const {
    return x == other.x && y == other.y && z == other.z;
  }

  bool operator!=(const Vertex& other) const { return !(*this == other); }

  Vertex operator+(const Vertex& other) const {
    return Vertex{x + other.x, y + other.y, z + other.z};
  }

  Vertex& operator+=(const Vertex& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vertex operator-(const Vertex& other) const {
    return Vertex{x - other.x, y - other.y, z - other.z};
  }

  Vertex& operator-=(const Vertex& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  Vertex operator-() const { return Vertex{-x, -y, -z}; }

  Vertex operator*(float s) const { return Vertex{x * s, y * s, z * s}; }

  Vertex& operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }

  Vertex operator/(float s) const { return Vertex{x / s, y / s, z / s}; }

  Vertex& operator/=(float s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }
};

inline Vertex operator*(float s, const Vertex& v) {
  return Vertex{v.x * s, v.y * s, v.z * s};
}
}  // namespace livision