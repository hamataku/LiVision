#pragma once

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#include "FastLS/utils.hpp"

namespace fastls {
class ObjectBase {
 public:
  virtual void AddMeshList(){};
  virtual void InitImpl(){};
  virtual void Draw(bgfx::ProgramHandle& program){};

  void Init() {
    if (!is_initialized_) {
      InitImpl();
    }
    is_initialized_ = true;
  }

  ObjectBase& SetColor(const utils::Color& color) {
    color_ = color;
    local_mtx_changed_ = true;
    return *this;
  }
  utils::Color GetColor() const { return color_; }

  ObjectBase& SetPos(const glm::vec3& pos) {
    pos_ = pos;
    local_mtx_changed_ = true;
    return *this;
  }
  glm::vec3 GetPos() const { return pos_; }

  ObjectBase& SetSize(const glm::vec2& size) {
    size_ = glm::vec3(size, 1.0F);
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetSize(const glm::vec3& size) {
    size_ = size;
    local_mtx_changed_ = true;
    return *this;
  }
  glm::vec3 GetSize() const { return size_; }

  ObjectBase& SetQuatRotation(const glm::quat& rotation) {
    quat_ = rotation;
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetDegRotation(const glm::vec3& euler) {
    glm::vec3 euler_rad = glm::radians(euler);
    quat_ = glm::quat(euler_rad);
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetRadRotation(const glm::vec3& euler_rad) {
    quat_ = glm::quat(euler_rad);
    local_mtx_changed_ = true;
    return *this;
  }
  glm::quat GetQuatRotation() const { return quat_; }

  ObjectBase& SetVisible(bool visible) {
    if (force_visible_) {
      std::cerr << "SetVisible: Force visible is set" << std::endl;
    }
    visible_ = visible;
    return *this;
  }
  bool IsVisible() const { return visible_; }

  ObjectBase& SetLidarVisible(bool visible) {
    lidar_visible_ = visible;
    return *this;
  }
  bool IsLidarVisible() const { return lidar_visible_; }
  ObjectBase& SetForceVisible(bool visible) {
    force_visible_ = visible;
    return *this;
  }
  bool IsForceVisible() const { return force_visible_; }

  void RegisterParentObject(ObjectBase* obj) { parent_object_ = obj; }

  glm::mat4 GetGlobalMatrix() const { return global_mtx_; }

  glm::mat4 GetLocalMatrix() const { return local_mtx_; }

  void ForceSetGlobalMatrix(const glm::mat4& mtx) { global_mtx_ = mtx; }

  void UpdateMatrix() {
    if (local_mtx_changed_) {
      local_mtx_ = glm::translate(glm::mat4(1.0F), pos_) *
                   glm::mat4_cast(quat_) * glm::scale(glm::mat4(1.0F), size_);
      local_mtx_changed_ = false;
    }

    // update global_mtx_
    if (parent_object_) {
      parent_object_->UpdateMatrix();
      global_mtx_ = parent_object_->global_mtx_ * local_mtx_;
    } else {
      global_mtx_ = local_mtx_;
    }
  }

 protected:
  glm::mat4 global_mtx_;
  utils::Color color_{1.0F, 1.0F, 1.0F, 1.0F};
  glm::vec3 pos_ = glm::vec3(0.0F, 0.0F, 0.0F);
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  glm::quat quat_ = glm::quat(1.0F, 0.0F, 0.0F, 0.0F);
  bool force_visible_ = false;  // visible if v key is pressed

 private:
  bool local_mtx_changed_ = true;
  glm::mat4 local_mtx_;
  bool visible_ = true;
  bool lidar_visible_ = true;
  ObjectBase* parent_object_ = nullptr;
  bool is_initialized_ = false;
};
}  // namespace fastls