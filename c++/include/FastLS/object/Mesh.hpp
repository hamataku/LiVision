#pragma once

#include "../SimLidar.hpp"
#include "FastLS/STLParser.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Mesh : public ObjectBase {
 public:
  explicit Mesh(const std::string& path) {
    ParseBinarySTL(path, vertices_, indices_);
  }

  void Init() override {
    vbh_ = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices_.data(),
                      vertices_.size() * sizeof(utils::Vec3Struct)),
        utils::vec3_vlayout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(indices_.data(), indices_.size() * sizeof(uint32_t)),
        BGFX_BUFFER_INDEX32);
  }

  void AddMeshList() override {
    utils::Mat mtx;
    CalcMtx(mtx);
    sim_lidar.AddMeshLists(vertices_, indices_, mtx);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    utils::Mat mtx;
    CalcMtx(mtx);
    bgfx::setTransform(mtx.data());

    bgfx::setVertexBuffer(0, vbh_);
    bgfx::setIndexBuffer(ibh_);
    bgfx::submit(0, program);
  }

 private:
  bgfx::VertexBufferHandle vbh_ = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh_ = BGFX_INVALID_HANDLE;
  std::vector<utils::Vec3Struct> vertices_;
  std::vector<uint32_t> indices_;
};
}  // namespace fastls
