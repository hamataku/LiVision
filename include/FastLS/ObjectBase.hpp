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

  virtual utils::MeshView GetMeshView() {
    return utils::MeshView{utils::none_vertices, utils::none_indices};
  }
  virtual void InitImpl() {};
  virtual void Draw(bgfx::ProgramHandle& program) { (void)program; };

  void Init() {
    if (!is_initialized_) {
      InitImpl();
    }
    is_initialized_ = true;
  }

  ObjectBase& SetColorSpec(const utils::ColorSpec& s) {
    color_spec_ = s;
    local_mtx_changed_ = true;
    return *this;
  }
  utils::ColorSpec GetColorSpec() const { return color_spec_; }

  // Apply color-related uniforms for the shader (call before submit)
  void ApplyColorUniforms() {
    bgfx::setUniform(utils::u_color, &color_spec_.base);
    float mode_val[4] = {static_cast<float>(static_cast<int>(color_spec_.mode)),
                         0.0F, 0.0F, 0.0F};
    float rparams[4] = {
        color_spec_.rainbow.direction.x, color_spec_.rainbow.direction.y,
        color_spec_.rainbow.direction.z, color_spec_.rainbow.delta};
    bgfx::setUniform(utils::u_color_mode, mode_val);
    bgfx::setUniform(utils::u_rainbow_params, rparams);
  }

  ObjectBase& SetPos(const glm::dvec3& pos) {
    pos_ = pos;
    local_mtx_changed_ = true;
    return *this;
  }
  glm::dvec3 GetLocalPos() const { return pos_; }
  glm::dvec3 GetGlobalPos() {
    UpdateMatrix();
    return glm::dvec3(global_mtx_[3][0], global_mtx_[3][1], global_mtx_[3][2]);
  }

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
  // NOLINTNEXTLINE
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

  ObjectBase& SetLidarDynamicObserve(bool is_lidar_dynamic) {
    is_lidar_dynamic_ = is_lidar_dynamic;
    return *this;
  }
  // NOLINTNEXTLINE
  bool IsLidarDynamicObserve() const { return is_lidar_dynamic_; }

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
  utils::ColorSpec color_spec_ = utils::white;
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
  bool is_lidar_dynamic_ = false;
};
}  // namespace fastls