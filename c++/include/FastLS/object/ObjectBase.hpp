#pragma once

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FastLS/utils.hpp"

namespace fastls {
class ObjectBase {
 public:
  virtual void AddMeshList(){};
  virtual void Init(){};
  virtual void Draw(bgfx::ProgramHandle& program) = 0;

  ObjectBase& SetColor(const utils::Color& color) {
    color_ = color;
    mtx_changed_ = true;
    return *this;
  }
  utils::Color GetColor() const { return color_; }

  ObjectBase& SetPos(const glm::vec3& pos) {
    pos_ = pos;
    mtx_changed_ = true;
    return *this;
  }
  glm::vec3 GetPos() const { return pos_; }

  ObjectBase& SetSize(const glm::vec2& size) {
    size_ = glm::vec3(size, 1.0F);
    mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetSize(const glm::vec3& size) {
    size_ = size;
    mtx_changed_ = true;
    return *this;
  }
  glm::vec3 GetSize() const { return size_; }

  ObjectBase& SetQuatRotation(const glm::quat& rotation) {
    quat_ = rotation;
    mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetDegRotation(const glm::vec3& euler) {
    glm::vec3 euler_rad = glm::radians(euler);
    quat_ = glm::quat(euler_rad);
    mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetRadRotation(const glm::vec3& euler_rad) {
    quat_ = glm::quat(euler_rad);
    mtx_changed_ = true;
    return *this;
  }
  glm::quat GetRotation() const { return quat_; }

 protected:
  void CalcMtx() {
    if (!mtx_changed_) {
      return;
    }
    mtx_ = glm::translate(glm::mat4(1.0F), pos_) * glm::mat4_cast(quat_) *
           glm::scale(glm::mat4(1.0F), size_);

    mtx_changed_ = false;
  }

  glm::mat4 mtx_;

  utils::Color color_{1.0F, 1.0F, 1.0F, 1.0F};
  glm::vec3 pos_ = glm::vec3(0.0F, 0.0F, 0.0F);
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  glm::quat quat_ = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);

 private:
  bool mtx_changed_ = true;
};
}  // namespace fastls