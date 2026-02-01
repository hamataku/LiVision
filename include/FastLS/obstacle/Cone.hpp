#pragma once

#include "LiVision/ObjectBase.hpp"

namespace livision {

class Cone : public ObjectBase {
 public:
  utils::MeshView GetMeshView() final {
    return utils::MeshView{.vertices = utils::cone_vertices,
                           .indices = utils::cone_indices};
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    ApplyColorUniforms();
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, utils::cone_vbh);
    bgfx::setIndexBuffer(utils::cone_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace livision
