#include "FastLS/utils.hpp"

#include <iostream>

#include "FastLS/file_ops.hpp"

namespace fastls::utils {
void Init() {
  // common
  u_color = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);

  vec2_vlayout.begin()
      .add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
      .end();

  vec3_vlayout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  vec4_vlayout.begin()
      .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
      .end();

  // cube
  cube_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(cube_vertices.data(),
                    cube_vertices.size() * sizeof(utils::Vec3Struct)),
      vec3_vlayout);
  cube_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(cube_index.data(), cube_index.size() * sizeof(uint16_t)));

  // line
  plane_vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(plane_vertices.data(),
                    plane_vertices.size() * sizeof(utils::Vec3Struct)),
      vec3_vlayout);
  plane_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(plane_index.data(), plane_index.size() * sizeof(uint16_t)));
}

void DeInit() {
  bgfx::destroy(u_color);
  bgfx::destroy(cube_vbh);
  bgfx::destroy(cube_ibh);
  bgfx::destroy(plane_vbh);
  bgfx::destroy(plane_ibh);
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