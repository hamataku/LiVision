#include "FastLS/utils.hpp"

#include <bgfx/defines.h>

#include <iostream>

#include "FastLS/file_ops.hpp"

namespace fastls::utils {
void Init() {
  // common
  u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
  u_color_mode = bgfx::createUniform("u_color_mode", bgfx::UniformType::Vec4);
  u_rainbow_params =
      bgfx::createUniform("u_rainbow_params", bgfx::UniformType::Vec4);

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
  CreateSphereBuffer();
  CreateConeBuffer();
}

void DeInit() {
  bgfx::destroy(u_color);
  bgfx::destroy(u_color_mode);
  bgfx::destroy(u_rainbow_params);
  bgfx::destroy(cube_vbh);
  bgfx::destroy(cube_ibh);
  bgfx::destroy(plane_vbh);
  bgfx::destroy(plane_ibh);
  bgfx::destroy(cylinder_vbh);
  bgfx::destroy(cylinder_ibh);
  bgfx::destroy(sphere_vbh);
  bgfx::destroy(sphere_ibh);
}

void CreateSphereBuffer() {
  // 球の分割数と半径を定義
  constexpr int kLatitudeBands = 32;
  constexpr int kLongitudeBands = 32;
  constexpr float kRadius = 0.5F;

  // 頂点データを生成
  for (int i = 0; i <= kLatitudeBands; ++i) {
    // 緯度 (Z軸からの角度 phi)
    float lat_angle = M_PI * static_cast<float>(i) / kLatitudeBands;
    float sin_lat = std::sin(lat_angle);
    float cos_lat = std::cos(lat_angle);

    for (int j = 0; j <= kLongitudeBands; ++j) {
      // 経度 (XY平面上の角度 theta)
      float long_angle = 2.0F * M_PI * static_cast<float>(j) / kLongitudeBands;
      float sin_long = std::sin(long_angle);
      float cos_long = std::cos(long_angle);

      // 球座標系から直交座標系へ変換
      glm::vec3 vertex;
      vertex.x = kRadius * sin_lat * cos_long;
      vertex.y = kRadius * sin_lat * sin_long;
      vertex.z = kRadius * cos_lat;
      sphere_vertices.push_back(vertex);
    }
  }

  // インデックスデータを生成
  for (int i = 0; i < kLatitudeBands; ++i) {
    for (int j = 0; j < kLongitudeBands; ++j) {
      uint32_t first = (i * (kLongitudeBands + 1)) + j;
      uint32_t second = first + kLongitudeBands + 1;

      // 1つ目の三角形
      sphere_indices.push_back(first);
      sphere_indices.push_back(second);
      sphere_indices.push_back(first + 1);

      // 2つ目の三角形
      sphere_indices.push_back(second);
      sphere_indices.push_back(second + 1);
      sphere_indices.push_back(first + 1);
    }
  }

  // bgfxの頂点バッファとインデックスバッファを作成
  sphere_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(sphere_vertices.data(),
                    sphere_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);
  sphere_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(sphere_indices.data(),
                    sphere_indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);
}

void CreateCylinderBuffer() {
  constexpr int kPoly = 32;
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

void CreateConeBuffer() {
  // Implementation for creating cone vertex and index buffers
  constexpr int kPoly = 16;
  constexpr float kRadius = 0.5F;
  constexpr float kHeight = 1.0F;

  cone_vertices.clear();
  cone_indices.clear();

  // Apex at +Z/2, base center at -Z/2
  cone_vertices.emplace_back(0.0F, 0.0F, kHeight * 0.5F);  // apex (idx 0)
  cone_vertices.emplace_back(0.0F, 0.0F,
                             -kHeight * 0.5F);  // base center (idx 1)

  // base rim vertices start at index 2
  for (int i = 0; i < kPoly; ++i) {
    float theta = glm::radians(360.0F * static_cast<float>(i) / kPoly);
    cone_vertices.emplace_back(kRadius * std::cos(theta),
                               kRadius * std::sin(theta), -kHeight * 0.5F);
  }

  const uint32_t apex_idx = 0;
  const uint32_t base_center_idx = 1;
  const uint32_t rim_start = 2;

  // side triangles (apex, rim_i, rim_{i+1})
  for (int i = 0; i < kPoly; ++i) {
    uint32_t i0 = rim_start + i;
    uint32_t i1 = rim_start + ((i + 1) % kPoly);
    cone_indices.push_back(apex_idx);
    cone_indices.push_back(i0);
    cone_indices.push_back(i1);
  }

  // base triangles (base_center, rim_{i+1}, rim_i) so normal points -Z
  for (int i = 0; i < kPoly; ++i) {
    uint32_t i0 = rim_start + i;
    uint32_t i1 = rim_start + ((i + 1) % kPoly);
    cone_indices.push_back(base_center_idx);
    cone_indices.push_back(i1);
    cone_indices.push_back(i0);
  }

  // create cone buffers
  cone_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(cone_vertices.data(),
                    cone_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);
  cone_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(cone_indices.data(),
                    cone_indices.size() * sizeof(uint32_t)),
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