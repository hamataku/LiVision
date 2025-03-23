#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Cylinder : public ObjectBase {
 public:
  void AddMeshList() final {
    CalcMtx();
    sim_lidar.AddMeshLists(utils::cylinder_vertices, utils::cylinder_indices,
                           mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    CalcMtx();
    bgfx::setTransform(glm::value_ptr(mtx_));

    bgfx::setVertexBuffer(0, utils::cylinder_vbh);
    bgfx::setIndexBuffer(utils::cylinder_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
