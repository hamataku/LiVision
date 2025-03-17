#pragma once

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <iostream>

namespace fastgl {
class ObjectBase {
 public:
  virtual void Init(){};
  virtual void Draw(bgfx::ProgramHandle& program) = 0;

  void SetPos(const glm::vec3& pos) { pos_ = pos; }
  void SetRotation(const glm::quat& rotation) { rotation_ = rotation; }

  ~ObjectBase() {
    bgfx::destroy(vbh_);
    bgfx::destroy(ibh_);
  }

 protected:
  glm::vec3 pos_ = glm::vec3(0.0F, 0.0F, 0.0F);
  glm::quat rotation_ = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);

  bgfx::VertexBufferHandle vbh_ = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh_ = BGFX_INVALID_HANDLE;
};
}  // namespace fastgl