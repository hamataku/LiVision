#include "FastLS/utils.hpp"

#include <bgfx/defines.h>

#include <iostream>

#include "FastLS/file_ops.hpp"

namespace fastls::utils {
void Init() {
  // common
  u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);

  float_vlayout.begin()
      .add(bgfx::Attrib::Position, 1, bgfx::AttribType::Float)
      .end();

  vec2_vlayout.begin()
      .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
      .end();

  vec3_vlayout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  vec4_vlayout.begin()
      .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
      .end();

  mat4_vlayout.begin()
      .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
      .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
      .end();

  // cube
  cube_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(cube_vertices.data(),
                    cube_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);
  cube_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(cube_indices.data(),
                    cube_indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);

  // line
  plane_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(plane_vertices.data(),
                    plane_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);
  plane_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(plane_indices.data(),
                    plane_indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);

  CreateCylinderBuffer();
}

void DeInit() {
  bgfx::destroy(u_color);
  bgfx::destroy(cube_vbh);
  bgfx::destroy(cube_ibh);
  bgfx::destroy(plane_vbh);
  bgfx::destroy(plane_ibh);
}

void CreateCylinderBuffer() {
  constexpr int kPoly = 16;
  constexpr float kSize = 0.5F;

  // make top cylinder_vertices
  for (int i = 0; i < kPoly; i++) {
    float theta = glm::radians(360.0F / kPoly * i);
    // top
    cylinder_vertices.emplace_back(kSize * std::cos(theta),
                                   kSize * std::sin(theta), kSize);
  }

  // make bottom cylinder_vertices
  for (int i = 0; i < kPoly; i++) {
    float theta = glm::radians(360.0F / kPoly * i);
    // bottom
    cylinder_vertices.emplace_back(kSize * std::cos(theta),
                                   kSize * std::sin(theta), -kSize);
  }

  // make top_indices
  for (int i = 0; i < kPoly; i++) {
    cylinder_indices.push_back(0);
    cylinder_indices.push_back(i);
    cylinder_indices.push_back((i + 1) % kPoly);
  }

  // make bottom_indices
  for (int i = 0; i < kPoly; i++) {
    cylinder_indices.push_back(kPoly);
    cylinder_indices.push_back(kPoly + ((i + 1) % kPoly));
    cylinder_indices.push_back(kPoly + i);
  }

  // make side_indices
  for (int i = 0; i < kPoly; i++) {
    cylinder_indices.push_back(i);
    cylinder_indices.push_back(kPoly + i);
    cylinder_indices.push_back((i + 1) % kPoly);

    cylinder_indices.push_back((i + 1) % kPoly);
    cylinder_indices.push_back(kPoly + i);
    cylinder_indices.push_back(kPoly + ((i + 1) % kPoly));
  }

  // cylinder
  cylinder_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(cylinder_vertices.data(),
                    cylinder_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);
  cylinder_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(cylinder_indices.data(),
                    cylinder_indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);
}

bgfx::ShaderHandle CreateShader(const std::string& path, const char* name) {
  std::string shader;
  if (!file_ops::ReadFile(path, shader)) {
    std::cerr << "Could not find compute shader" << std::endl;
    return BGFX_INVALID_HANDLE;
  }
  const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
  const bgfx::ShaderHandle handle = bgfx::createShader(mem);
  bgfx::setName(handle, name);
  return handle;
}
}  // namespace fastls::utils