#pragma once

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class Cylinder : public ObjectBase {
 public:
  utils::MeshView GetMeshView() final {
    return utils::MeshView{.vertices = utils::cylinder_vertices,
                           .indices = utils::cylinder_indices};
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
