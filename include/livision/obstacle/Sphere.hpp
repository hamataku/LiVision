#pragma once

#include "livision/ObjectBase.hpp"

namespace livision {

class Sphere : public ObjectBase {
 public:
  utils::MeshView GetMeshView() final {
    return utils::MeshView{.vertices = utils::sphere_vertices,
                           .indices = utils::sphere_indices};
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setState(utils::kAlphaState);
    ApplyColorUniforms();
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, utils::sphere_vbh);
    bgfx::setIndexBuffer(utils::sphere_ibh);
    bgfx::submit(0, program);
  }
};
}  // namespace livision
