#pragma once

#include "../SimLidar.hpp"
#include "FastLS/MeshParser.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Mesh : public ObjectBase {
 public:
  explicit Mesh(const std::string& path) {
    mesh_parser::ParseMeshFile(path, vertices_, indices_);
  }

  void Init() override {
    vbh_ = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices_.data(), vertices_.size() * sizeof(glm::vec3)),
        utils::vec3_vlayout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(indices_.data(), indices_.size() * sizeof(uint32_t)),
        BGFX_BUFFER_INDEX32);
  }

  void AddMeshList() override {
    sim_lidar.AddMeshLists(vertices_, indices_, global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    bgfx::setTransform(glm::value_ptr(global_mtx_));

    bgfx::setVertexBuffer(0, vbh_);
    bgfx::setIndexBuffer(ibh_);
    bgfx::submit(0, program);
  }

 private:
  bgfx::VertexBufferHandle vbh_ = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh_ = BGFX_INVALID_HANDLE;
  std::vector<glm::vec3> vertices_;
  std::vector<uint32_t> indices_;
};
}  // namespace fastls
