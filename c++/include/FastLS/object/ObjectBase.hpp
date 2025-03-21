#pragma once

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "FastLS/utils.hpp"

namespace fastls {
class ObjectBase {
 public:
  virtual void AddMeshList() {};
  virtual void Init() {};
  virtual void Draw(bgfx::ProgramHandle& program) = 0;

  void SetColor(const utils::Color& color) { color_ = color; }

  void SetPos(const glm::vec3& pos) { pos_ = pos; }

  void SetSize(const glm::vec2& size) { size_ = glm::vec3(size, 1.0F); }
  void SetSize(const glm::vec3& size) { size_ = size; }

  void SetRotation(const glm::quat& rotation) { rotation_ = rotation; }
  void SetRotation(const glm::vec3& euler) { rotation_ = glm::quat(euler); }

 protected:
  void CalcMtx(utils::Mat& mtx) const {
    bx::mtxScale(mtx.data(), size_.x, size_.y, size_.z);
    mtx[12] = pos_.x;
    mtx[13] = pos_.y;
    mtx[14] = pos_.z;
  }

  utils::Color color_{1.0F, 0.0F, 0.0F, 0.0F};
  glm::vec3 pos_ = glm::vec3(0.0F, 0.0F, 0.0F);
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  glm::quat rotation_ = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
};
}  // namespace fastls