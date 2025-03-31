#pragma once

#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Plane : public ObjectBase {
 public:
  void AddMeshList() final {
    lidar_sim.AddMeshLists(utils::plane_vertices, utils::plane_indices,
                           global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);

    bgfx::setTransform(glm::value_ptr(global_mtx_));

    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }
};

}  // namespace fastls
