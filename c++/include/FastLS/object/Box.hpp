#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Box : public ObjectBase {
 public:
  void AddMeshList() override {
    utils::Mat mtx;
    CalcMtx(mtx);
    sim_lidar.AddMeshLists(utils::cube_vertices, utils::cube_indices, mtx);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    utils::Mat mtx;
    CalcMtx(mtx);
    bgfx::setTransform(mtx.data());

    bgfx::setVertexBuffer(0, utils::cube_vbh);
    bgfx::setIndexBuffer(utils::cube_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
