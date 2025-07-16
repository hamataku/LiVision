#pragma once
#include <bgfx/bgfx.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace fastls::utils {

// Declare types
struct Color {
  float r;
  float g;
  float b;
  float a;

  bool operator==(const Color& c) const {
    return r == c.r && g == c.g && b == c.b && a == c.a;
  }

  constexpr Color(float r, float g, float b, float a)
      : r(r), g(g), b(b), a(a) {}
  constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0F) {}

  Color Alpha(float a2) const { return {r, g, b, a2}; }
};

// colors
// NOLINTBEGIN
inline constexpr Color white{1.0F, 1.0F, 1.0F};
inline constexpr Color black{0.0F, 0.0F, 0.0F};
inline constexpr Color red{1.0F, 0.0F, 0.0F};
inline constexpr Color green{0.0F, 1.0F, 0.0F};
inline constexpr Color blue{0.0F, 0.0F, 1.0F};
inline constexpr Color yellow{1.0F, 1.0F, 0.0F};
inline constexpr Color cyan{0.0F, 1.0F, 1.0F};
inline constexpr Color magenta{1.0F, 0.0F, 1.0F};
inline constexpr Color gray{0.5F, 0.5F, 0.5F};
inline constexpr Color light_gray{0.75F, 0.75F, 0.75F};
inline constexpr Color dark_gray{0.25F, 0.25F, 0.25F};

// Uniforms
inline bgfx::UniformHandle u_color;

// State
inline constexpr uint64_t kAlphaState =
    BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA;

// NOLINTEND

// Cube
inline bgfx::VertexBufferHandle cube_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle cube_ibh = BGFX_INVALID_HANDLE;
inline std::vector<glm::vec3> cube_vertices{
    {-0.5F, 0.5F, 0.5F},   {0.5F, 0.5F, 0.5F},   {-0.5F, -0.5F, 0.5F},
    {0.5F, -0.5F, 0.5F},   {-0.5F, 0.5F, -0.5F}, {0.5F, 0.5F, -0.5F},
    {-0.5F, -0.5F, -0.5F}, {0.5F, -0.5F, -0.5F},
};
inline std::vector<uint32_t> cube_indices{
    0, 2, 1, 1, 2, 3, 4, 5, 6, 5, 7, 6, 0, 4, 2, 4, 6, 2,
    1, 3, 5, 5, 3, 7, 0, 1, 4, 4, 1, 5, 2, 6, 3, 6, 7, 3,
};

// Plane
inline bgfx::VertexBufferHandle plane_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle plane_ibh = BGFX_INVALID_HANDLE;
inline std::vector<glm::vec3> plane_vertices{
    {-0.5F, 0.5F, 0.0F},   // top-left
    {0.5F, 0.5F, 0.0F},    // top-right
    {-0.5F, -0.5F, 0.0F},  // bottom-left
    {0.5F, -0.5F, 0.0F}    // bottom-right
};
inline std::vector<uint32_t> plane_indices{0, 1, 2, 1, 3, 2, 0, 2, 1, 1, 2, 3};

// Cylinder
inline bgfx::VertexBufferHandle cylinder_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle cylinder_ibh = BGFX_INVALID_HANDLE;
inline std::vector<glm::vec3> cylinder_vertices{};
inline std::vector<uint32_t> cylinder_indices{};

// Vertex Layout
inline bgfx::VertexLayout float_vlayout;
inline bgfx::VertexLayout vec2_vlayout;
inline bgfx::VertexLayout vec3_vlayout;
inline bgfx::VertexLayout vec4_vlayout;
inline bgfx::VertexLayout mat4_vlayout;

// Functions
void Init();
void DeInit();
void CreateCylinderBuffer();
bgfx::ShaderHandle CreateShader(const std::string& path, const char* name);
}  // namespace fastls::utils