#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Cylinder : public ObjectBase {
 public:
  void AddMeshList() final {
    sim_lidar.AddMeshLists(utils::cylinder_vertices, utils::cylinder_indices,
                           global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);

    bgfx::setTransform(glm::value_ptr(global_mtx_));

    bgfx::setVertexBuffer(0, utils::cylinder_vbh);
    bgfx::setIndexBuffer(utils::cylinder_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
