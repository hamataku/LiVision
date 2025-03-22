#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Plane : public ObjectBase {
 public:
  void AddMeshList() override {
    utils::Mat mtx;
    CalcMtx(mtx);
    sim_lidar.AddMeshLists(utils::plane_vertices, utils::plane_indices, mtx);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    utils::Mat mtx;
    CalcMtx(mtx);
    bgfx::setTransform(mtx.data());

    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }
};

}  // namespace fastls
