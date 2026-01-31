#pragma once

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {

class Box : public ObjectBase {
 public:
  utils::MeshView GetMeshView() final {
    return utils::MeshView{.vertices = utils::cube_vertices,
                           .indices = utils::cube_indices};
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, utils::cube_vbh);
    bgfx::setIndexBuffer(utils::cube_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace fastls
