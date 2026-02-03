#pragma once

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <memory>

#include "Color.hpp"
#include "MeshData.hpp"

namespace livision {

class Renderer;

class ObjectBase {
 public:
  virtual ~ObjectBase() = default;
  virtual void InitImpl() {}
  virtual void Draw(Renderer& renderer) {}

  void Init() {
    if (!is_initialized_) {
      InitImpl();
    }
    is_initialized_ = true;
  }

  // 位置・サイズ・回転
  ObjectBase& SetPos(const Eigen::Vector3d& pos) {
    pos_ = pos;
    local_mtx_changed_ = true;
    return *this;
  }
  Eigen::Vector3d GetGlobalPos() {
    UpdateMatrix();
    return global_mtx_.translation();
  }

  ObjectBase& SetSize(const Eigen::Vector3d& size) {
    size_ = size;
    local_mtx_changed_ = true;
    return *this;
  }
  ObjectBase& SetQuatRotation(const Eigen::Quaterniond& q) {
    quat_ = q;
    local_mtx_changed_ = true;
    return *this;
  }

  ObjectBase& SetVisible(bool visible) {
    visible_ = visible;
    return *this;
  }

  ObjectBase& SetColor(const Color& color) {
    color_ = color;
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

  // NOLINTNEXTLINE
  void UpdateMatrix() {
    if (local_mtx_changed_) {
      // EigenではAffine3dを使って平行移動・回転・スケールを順番に合成
      Eigen::Affine3d translation = Eigen::Affine3d(Eigen::Translation3d(pos_));
      // NOLINTNEXTLINE
      Eigen::Affine3d rotation = Eigen::Affine3d(quat_);
      Eigen::Affine3d scale = Eigen::Affine3d(Eigen::Scaling(size_));

      local_mtx_ = translation * rotation * scale;
      local_mtx_changed_ = false;
    }

    // グローバル行列の更新
    if (parent_object_) {
      parent_object_->UpdateMatrix();
      global_mtx_ = parent_object_->global_mtx_ * local_mtx_;
    } else {
      global_mtx_ = local_mtx_;
    }
  }

  void RegisterParentObject(ObjectBase* obj) { parent_object_ = obj; }

 protected:
  Eigen::Affine3d global_mtx_ = Eigen::Affine3d::Identity();
  Eigen::Affine3d local_mtx_ = Eigen::Affine3d::Identity();
  Eigen::Vector3d pos_ = Eigen::Vector3d::Zero();
  Eigen::Vector3d size_ = Eigen::Vector3d::Ones();
  Eigen::Quaterniond quat_ = Eigen::Quaterniond::Identity();

  Color color_ = color::white;

  bool local_mtx_changed_ = true;
  bool is_initialized_ = false;

  bool visible_ = true;
  ObjectBase* parent_object_ = nullptr;

  std::shared_ptr<MeshData> mesh_;
};

// class ObjectBase {
//  public:
//   ObjectBase() = default;
//   virtual ~ObjectBase() = default;

//   virtual void InitImpl() {};
//   virtual void Draw(Renderer& renderer) { (void)renderer; };

//   void Init() {
//     if (!is_initialized_) {
//       InitImpl();
//     }
//     is_initialized_ = true;
//   }

//   ObjectBase& SetColorSpec(const utils::ColorSpec& s) {
//     color_spec_ = s;
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   utils::ColorSpec GetColorSpec() const { return color_spec_; }

//   ObjectBase& SetPos(const glm::dvec3& pos) {
//     pos_ = pos;
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   glm::dvec3 GetLocalPos() const { return pos_; }
//   glm::dvec3 GetGlobalPos() {
//     UpdateMatrix();
//     return glm::dvec3(global_mtx_[3][0], global_mtx_[3][1],
//     global_mtx_[3][2]);
//   }

//   ObjectBase& SetSize(const glm::dvec2& size) {
//     size_ = glm::vec3(size, 1.0F);
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   ObjectBase& SetSize(const glm::dvec3& size) {
//     size_ = size;
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   glm::vec3 GetSize() const { return size_; }

//   ObjectBase& SetQuatRotation(const glm::dquat& rotation) {
//     quat_ = rotation;
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   ObjectBase& SetDegRotation(const glm::dvec3& euler) {
//     glm::dvec3 euler_rad = glm::radians(euler);
//     quat_ = glm::dquat(euler_rad);
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   ObjectBase& SetRadRotation(const glm::dvec3& euler_rad) {
//     quat_ = glm::dquat(euler_rad);
//     local_mtx_changed_ = true;
//     return *this;
//   }
//   glm::dquat GetQuatRotation() const { return quat_; }

//   ObjectBase& SetVisible(bool visible) {
//     visible_ = visible;
//     return *this;
//   }
//   // NOLINTNEXTLINE
//   bool IsVisible() const {
//     if (parent_object_ && visible_) {
//       // If parent object is set, visibility is determined by parent
//       return parent_object_->IsVisible();
//     }
//     return visible_;
//   }

//   void RegisterParentObject(ObjectBase* obj) { parent_object_ = obj; }

//   glm::dmat4 GetGlobalMatrix() const { return global_mtx_; }

//   glm::dmat4 GetLocalMatrix() const { return local_mtx_; }

//   void ForceSetGlobalMatrix(const glm::dmat4& mtx) { global_mtx_ = mtx; }

//   // NOLINTNEXTLINE
//   void UpdateMatrix() {
//     if (local_mtx_changed_) {
//       local_mtx_ = glm::translate(glm::dmat4(1.0), pos_) *
//                    glm::mat4_cast(quat_) * glm::scale(glm::dmat4(1.0),
//                    size_);
//       local_mtx_changed_ = false;
//     }

//     // update global_mtx_
//     if (parent_object_) {
//       parent_object_->UpdateMatrix();
//       global_mtx_ = parent_object_->global_mtx_ * local_mtx_;
//     } else {
//       global_mtx_ = local_mtx_;
//     }
//   }

//  protected:
//   glm::dmat4 global_mtx_;
//   utils::ColorSpec color_spec_ = utils::white;
//   glm::dvec3 pos_ = glm::vec3(0.0, 0.0, 0.0);
//   glm::dvec3 size_ = glm::vec3(1.0, 1.0, 1.0);
//   glm::dquat quat_ = glm::dquat(1.0, 0.0, 0.0, 0.0);

//  private:
//   // Apply color-related uniforms for the shader (call before submit)
//   void ApplyColorUniforms() {
//     bgfx::setUniform(utils::u_color, &color_spec_.base);
//     float mode_val[4] =
//     {static_cast<float>(static_cast<int>(color_spec_.mode)),
//                          0.0F, 0.0F, 0.0F};
//     float rparams[4] = {
//         color_spec_.rainbow.direction.x, color_spec_.rainbow.direction.y,
//         color_spec_.rainbow.direction.z, color_spec_.rainbow.delta};
//     bgfx::setUniform(utils::u_color_mode, mode_val);
//     bgfx::setUniform(utils::u_rainbow_params, rparams);
//   }

//   bool local_mtx_changed_ = true;
//   glm::dmat4 local_mtx_;
//   bool visible_ = true;
//   ObjectBase* parent_object_ = nullptr;
//   bool is_initialized_ = false;
// };
}  // namespace livision