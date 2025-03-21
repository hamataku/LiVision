#pragma once
#include <bgfx/bgfx.h>

#include <array>
#include <string>
#include <vector>

namespace fastls::utils {

// Declare types
struct Vec3Struct {
  float x;
  float y;
  float z;
};

struct Vec4Struct {
  float x;
  float y;
  float z;
  float w;
};

using Mat = std::array<float, 16>;

struct Color {
  float r;
  float g;
  float b;
  float a;
};

// Uniforms
inline bgfx::UniformHandle u_color;

// Cube
inline bgfx::VertexBufferHandle cube_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle cube_ibh = BGFX_INVALID_HANDLE;
inline std::vector<utils::Vec3Struct> cube_vertices{
    {-0.5F, 0.5F, 0.5F},   {0.5F, 0.5F, 0.5F},   {-0.5F, -0.5F, 0.5F},
    {0.5F, -0.5F, 0.5F},   {-0.5F, 0.5F, -0.5F}, {0.5F, 0.5F, -0.5F},
    {-0.5F, -0.5F, -0.5F}, {0.5F, -0.5F, -0.5F},
};
inline std::vector<uint16_t> cube_index{
    0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
    1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
};

// Plane
inline bgfx::VertexBufferHandle plane_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle plane_ibh = BGFX_INVALID_HANDLE;
inline std::vector<utils::Vec3Struct> plane_vertices{
    {-0.5F, 0.5F, 0.0F},   // top-left
    {0.5F, 0.5F, 0.0F},    // top-right
    {-0.5F, -0.5F, 0.0F},  // bottom-left
    {0.5F, -0.5F, 0.0F}    // bottom-right
};
inline std::vector<uint16_t> plane_index{0, 1, 2, 1, 3, 2, 0, 2, 1, 1, 2, 3};

// Vertex Layout
inline bgfx::VertexLayout vec2_vlayout;
inline bgfx::VertexLayout vec3_vlayout;
inline bgfx::VertexLayout vec4_vlayout;

// Functions
void Init();
void DeInit();
bgfx::ShaderHandle CreateShader(const std::string& path, const char* name);
}  // namespace fastls::utils