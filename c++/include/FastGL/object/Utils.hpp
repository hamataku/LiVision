#pragma once
#include <bgfx/bgfx.h>

namespace fastgl::utils {
struct PosVertex {
  float x;
  float y;
  float z;
};

inline bgfx::UniformHandle u_color;

inline bgfx::VertexBufferHandle cube_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle cube_ibh = BGFX_INVALID_HANDLE;

inline bgfx::VertexBufferHandle plane_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle plane_ibh = BGFX_INVALID_HANDLE;

inline void Init() {
  // common
  u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);

  bgfx::VertexLayout pos_vert_layout;
  pos_vert_layout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  // cube
  static constexpr utils::PosVertex kCubeVertices[8] = {
      {-0.5F, 0.5F, 0.5F},   {0.5F, 0.5F, 0.5F},   {-0.5F, -0.5F, 0.5F},
      {0.5F, -0.5F, 0.5F},   {-0.5F, 0.5F, -0.5F}, {0.5F, 0.5F, -0.5F},
      {-0.5F, -0.5F, -0.5F}, {0.5F, -0.5F, -0.5F},
  };
  static constexpr uint16_t kCubeTriList[36] = {
      0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
      1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
  };

  cube_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(kCubeVertices, sizeof(kCubeVertices)), pos_vert_layout);
  cube_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(kCubeTriList, sizeof(kCubeTriList)));

  // line
  static constexpr utils::PosVertex kPlaneVertices[4] = {
      {-0.5F, 0.5F, 0.0F},   // top-left
      {0.5F, 0.5F, 0.0F},    // top-right
      {-0.5F, -0.5F, 0.0F},  // bottom-left
      {0.5F, -0.5F, 0.0F}    // bottom-right
  };

  static constexpr uint16_t kPlaneTriList[12] = {0, 1, 2, 1, 3, 2,
                                                 0, 2, 1, 1, 2, 3};

  plane_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(kPlaneVertices, sizeof(kPlaneVertices)), pos_vert_layout);
  plane_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(kPlaneTriList, sizeof(kPlaneTriList)));
}

inline void DeInit() {
  bgfx::destroy(u_color);
  bgfx::destroy(cube_vbh);
  bgfx::destroy(cube_ibh);
  bgfx::destroy(plane_vbh);
  bgfx::destroy(plane_ibh);
}
}  // namespace fastgl::utils