#pragma once

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "FastLS/utils.hpp"

namespace fastls {
class ObjectBase {
 public:
  ObjectBase() = default;
  virtual ~ObjectBase() = default;

  virtual void AddMeshList() {};
  virtual void InitImpl() {};
  virtual void Draw(bgfx::ProgramHandle& program) { (void)program; };

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

  ObjectBase& SetPos(const glm::dvec3& pos) {
    pos_ = pos;
    local_mtx_changed_ = true;
    return *this;
  }
  glm::dvec3 GetPos() const { return pos_; }

  ObjectBase& SetSize(const glm::dvec2& size) {
    size_ = glm::vec3(size, 1.0F);
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetSize(const glm::dvec3& size) {
    size_ = size;
    local_mtx_changed_ = true;
    return *this;
  }
  glm::vec3 GetSize() const { return size_; }

  ObjectBase& SetQuatRotation(const glm::dquat& rotation) {
    quat_ = rotation;
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetDegRotation(const glm::dvec3& euler) {
    glm::dvec3 euler_rad = glm::radians(euler);
    quat_ = glm::dquat(euler_rad);
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetRadRotation(const glm::dvec3& euler_rad) {
    quat_ = glm::dquat(euler_rad);
    local_mtx_changed_ = true;
    return *this;
  }
  glm::dquat GetQuatRotation() const { return quat_; }

  ObjectBase& SetVisible(bool visible) {
    visible_ = visible;
    return *this;
  }
  bool IsVisible() const {
    if (parent_object_ && visible_) {
      // If parent object is set, visibility is determined by parent
      return parent_object_->IsVisible();
    }
    return visible_;
  }

  ObjectBase& SetLidarVisible(bool visible) {
    lidar_visible_ = visible;
    return *this;
  }
  bool IsLidarVisible() const { return lidar_visible_; }

  void RegisterParentObject(ObjectBase* obj) { parent_object_ = obj; }

  glm::dmat4 GetGlobalMatrix() const { return global_mtx_; }

  glm::dmat4 GetLocalMatrix() const { return local_mtx_; }

  void ForceSetGlobalMatrix(const glm::dmat4& mtx) { global_mtx_ = mtx; }

  // NOLINTNEXTLINE
  void UpdateMatrix() {
    if (local_mtx_changed_) {
      local_mtx_ = glm::translate(glm::dmat4(1.0), pos_) *
                   glm::mat4_cast(quat_) * glm::scale(glm::dmat4(1.0), size_);
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
  glm::dmat4 global_mtx_;
  utils::Color color_{1.0F, 1.0F, 1.0F, 1.0F};
  glm::dvec3 pos_ = glm::vec3(0.0, 0.0, 0.0);
  glm::dvec3 size_ = glm::vec3(1.0, 1.0, 1.0);
  glm::dquat quat_ = glm::dquat(1.0, 0.0, 0.0, 0.0);

 private:
  bool local_mtx_changed_ = true;
  glm::dmat4 local_mtx_;
  bool visible_ = true;
  bool lidar_visible_ = true;
  ObjectBase* parent_object_ = nullptr;
  bool is_initialized_ = false;
};
}  // namespace fastls