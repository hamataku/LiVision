#pragma once

#include "FastLS/object/ObjectBase.hpp"
#include "FastLS/sim/LidarSim.hpp"

namespace fastls {

class Sphere : public ObjectBase {
 public:
  void AddMeshList() final {
    lidar_sim.AddMeshLists(utils::sphere_vertices, utils::sphere_indices,
                           global_mtx_);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);
    bgfx::setTransform(glm::value_ptr(global_mtx_));

    bgfx::setVertexBuffer(0, utils::sphere_vbh);
    bgfx::setIndexBuffer(utils::sphere_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
