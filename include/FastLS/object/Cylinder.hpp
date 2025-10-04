#pragma once

#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Cylinder : public ObjectBase {
 public:
  void AddMeshList() final {
    lidar_sim.AddMeshLists(utils::cylinder_vertices, utils::cylinder_indices,
                           global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, utils::cylinder_vbh);
    bgfx::setIndexBuffer(utils::cylinder_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
