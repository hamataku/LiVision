#pragma once

#include "../SimLidar.hpp"
#include "ObjectBase.hpp"

namespace fastls {

class Plane : public ObjectBase {
 public:
  void AddMeshList() final {
    sim_lidar.AddMeshLists(utils::plane_vertices, utils::plane_indices,
                           global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, &color_);

    bgfx::setTransform(glm::value_ptr(global_mtx_));

    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }
};

}  // namespace fastls
