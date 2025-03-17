#pragma once
#include <bgfx/bgfx.h>

namespace fastgl::utils {
struct PosVertex {
  float x;
  float y;
  float z;
};

inline bgfx::UniformHandle u_color;

inline void Init() {
  u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
}

inline void DeInit() { bgfx::destroy(u_color); }
}  // namespace fastgl::utils