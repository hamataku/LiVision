#pragma once

#include "livision/ObjectBase.hpp"

namespace livision {
class Plane : public ObjectBase {
 public:
  utils::MeshView GetMeshView() final {
    return utils::MeshView{.vertices = utils::plane_vertices,
                           .indices = utils::plane_indices};
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    ApplyColorUniforms();
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }
};

}  // namespace livision
