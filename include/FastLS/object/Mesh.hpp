#pragma once

#include "FastLS/MeshParser.hpp"
#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Mesh : public ObjectBase {
 public:
  explicit Mesh(const std::string& path) {
    mesh_parser::ParseMeshFile(path, vertices_, indices_);
  }

  void InitImpl() override {
    vbh_ = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices_.data(), vertices_.size() * sizeof(glm::vec3)),
        utils::vec3_vlayout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(indices_.data(), indices_.size() * sizeof(uint32_t)),
        BGFX_BUFFER_INDEX32);
  }

  void AddMeshList() override {
    lidar_sim.AddMeshLists(vertices_, indices_, global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

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
