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
// Color mode and rainbow params used by shaders
enum class ColorMode { Fixed = 0, Rainbow = 1 };

struct RainbowParams {
  glm::vec3 direction{1.0F, 0.0F, 0.0F};
  float delta = 1.0F;
};

// Combined color specification for objects
struct ColorSpec {
  Color base{1.0F, 1.0F, 1.0F, 1.0F};
  ColorMode mode = ColorMode::Fixed;
  RainbowParams rainbow{};

  ColorSpec(Color c) : base(c), mode(ColorMode::Fixed), rainbow() {};
  ColorSpec(RainbowParams rp)
      : base{1.0F, 0.0F, 0.0F, 1.0F}, mode(ColorMode::Rainbow), rainbow(rp) {};
};

inline const ColorSpec white{Color(1.0F, 1.0F, 1.0F)};
inline const ColorSpec black{Color(0.0F, 0.0F, 0.0F)};

inline const ColorSpec gray{Color(0.5F, 0.5F, 0.5F)};
inline const ColorSpec light_gray{Color(0.75F, 0.75F, 0.75F)};
inline const ColorSpec off_white{Color(0.9F, 0.9F, 0.9F)};
inline const ColorSpec dark_gray{Color(0.25F, 0.25F, 0.25F)};

inline const ColorSpec red{Color(1.0F, 0.0F, 0.0F)};
inline const ColorSpec green{Color(0.0F, 1.0F, 0.0F)};
inline const ColorSpec blue{Color(0.0F, 0.0F, 1.0F)};
inline const ColorSpec yellow{Color(1.0F, 1.0F, 0.0F)};
inline const ColorSpec cyan{Color(0.0F, 1.0F, 1.0F)};
inline const ColorSpec magenta{Color(1.0F, 0.0F, 1.0F)};

inline const ColorSpec orange{Color(0.95F, 0.45F, 0.10F)};
inline const ColorSpec teal{Color(0.10F, 0.65F, 0.65F)};
inline const ColorSpec olive{Color(0.65F, 0.70F, 0.20F)};
inline const ColorSpec violet{Color(0.65F, 0.40F, 0.90F)};
inline const ColorSpec rose{Color(0.95F, 0.35F, 0.45F)};
inline const ColorSpec sand{Color(0.90F, 0.80F, 0.55F)};

inline const ColorSpec rainbow_z{RainbowParams{{0.0F, 0.0F, 1.0F}, 0.1F}};

inline std::vector<ColorSpec> color_palette{
    red, green, blue, yellow, cyan, magenta, orange, teal, olive, rose, sand};

// Uniforms
inline bgfx::UniformHandle u_color;
inline bgfx::UniformHandle u_color_mode;
inline bgfx::UniformHandle u_rainbow_params;

// State
inline constexpr uint64_t kAlphaState =
    BGFX_STATE_DEFAULT | BGFX_STATE_BLEND_ALPHA;
inline constexpr uint64_t kPointState = kAlphaState | BGFX_STATE_PT_POINTS;
inline constexpr uint64_t kPointSpriteState =
    kAlphaState | BGFX_STATE_PT_TRISTRIP;

// NOLINTEND

struct MeshView {
  const std::vector<glm::vec3>& vertices;
  const std::vector<uint32_t>& indices;
};

// None
inline std::vector<uint32_t> none_indices{};
inline std::vector<glm::vec3> none_vertices{};

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

// Sphere
inline bgfx::VertexBufferHandle sphere_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle sphere_ibh = BGFX_INVALID_HANDLE;
inline std::vector<glm::vec3> sphere_vertices{};
inline std::vector<uint32_t> sphere_indices{};

// Cone
inline bgfx::VertexBufferHandle cone_vbh = BGFX_INVALID_HANDLE;
inline bgfx::IndexBufferHandle cone_ibh = BGFX_INVALID_HANDLE;
inline std::vector<glm::vec3> cone_vertices{};
inline std::vector<uint32_t> cone_indices{};

// Vertex Layout
inline bgfx::VertexLayout float_vlayout;
inline bgfx::VertexLayout vec2_vlayout;
inline bgfx::VertexLayout vec3_vlayout;
inline bgfx::VertexLayout vec4_vlayout;
inline bgfx::VertexLayout mat4_vlayout;

// Programs
inline bgfx::ProgramHandle point_program = BGFX_INVALID_HANDLE;

// Functions
void Init();
void DeInit();
void CreateCylinderBuffer();
void CreateSphereBuffer();
void CreateConeBuffer();
bgfx::ShaderHandle CreateShader(const std::string& path, const char* name);
}  // namespace fastls::utils