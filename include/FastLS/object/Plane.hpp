#pragma once

#include "FastLS/object/ObjectBase.hpp"

namespace fastls {
class Plane : public ObjectBase {
 public:
  utils::MeshView GetMeshView() final {
    return utils::MeshView{utils::plane_vertices, utils::plane_indices};
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    bgfx::setUniform(utils::u_color, &color_);
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }
};

}  // namespace fastls
