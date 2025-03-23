#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Box : public ObjectBase {
 public:
  void AddMeshList() override {
    CalcMtx();
    sim_lidar.AddMeshLists(utils::cube_vertices, utils::cube_indices, mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    CalcMtx();
    bgfx::setTransform(glm::value_ptr(mtx_));

    bgfx::setVertexBuffer(0, utils::cube_vbh);
    bgfx::setIndexBuffer(utils::cube_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
