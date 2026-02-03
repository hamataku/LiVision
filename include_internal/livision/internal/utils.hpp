#pragma once
#include <bgfx/bgfx.h>

#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace livision::utils {

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
void CreateSphereBuffer();
void CreateConeBuffer();

}  // namespace livision::utils