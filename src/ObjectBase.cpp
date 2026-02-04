#include "livision/ObjectBase.hpp"

#include "livision/Renderer.hpp"

namespace livision {
void ObjectBase::OnDraw(Renderer& renderer) {
  if (mesh_buf_)
    renderer.Submit(*mesh_buf_, global_mtx_, params_.color, params_.wire_color);
}

void ObjectBase::Init() {
  if (!is_initialized_) {
    OnInit();
  }
  is_initialized_ = true;
}

void ObjectBase::DeInit() {
  is_initialized_ = false;
  if (mesh_buf_) {
    mesh_buf_->Destroy();
    mesh_buf_.reset();
  }
}

ObjectBase& ObjectBase::SetParams(const Params& params) {
  params_ = params;
  local_mtx_changed_ = true;
  return *this;
}

ObjectBase& ObjectBase::SetPos(const Eigen::Vector3d& pos) {
  params_.pos = pos;
  local_mtx_changed_ = true;
  return *this;
}
ObjectBase& ObjectBase::SetPos(double x, double y, double z) {
  params_.pos = Eigen::Vector3d(x, y, z);
  local_mtx_changed_ = true;
  return *this;
}
ObjectBase& ObjectBase::SetScale(const Eigen::Vector3d& scale) {
  params_.scale = scale;
  local_mtx_changed_ = true;
  return *this;
}
ObjectBase& ObjectBase::SetScale(double x, double y, double z) {
  params_.scale = Eigen::Vector3d(x, y, z);
  local_mtx_changed_ = true;
  return *this;
}
ObjectBase& ObjectBase::SetQuatRotation(const Eigen::Quaterniond& q) {
  params_.quat = q;
  local_mtx_changed_ = true;
  return *this;
}
ObjectBase& ObjectBase::SetDegRotation(const Eigen::Vector3d& euler_deg) {
  Eigen::Vector3d euler_rad = euler_deg * M_PI / 180.0;
  Eigen::AngleAxisd roll(euler_rad.x(), Eigen::Vector3d::UnitX());
  Eigen::AngleAxisd pitch(euler_rad.y(), Eigen::Vector3d::UnitY());
  Eigen::AngleAxisd yaw(euler_rad.z(), Eigen::Vector3d::UnitZ());
  Eigen::Quaterniond q = yaw * pitch * roll;
  return SetQuatRotation(q);
}
ObjectBase& ObjectBase::SetColor(const Color& color) {
  params_.color = color;
  return *this;
}
ObjectBase& ObjectBase::SetWireColor(const Color& color) {
  params_.wire_color = color;
  return *this;
}

ObjectBase& ObjectBase::SetGlobalMatrix(const Eigen::Affine3d& mtx) {
  global_mtx_ = mtx;
  return *this;
}

// NOLINTNEXTLINE
bool ObjectBase::IsVisible() const {
  if (parent_object_ && visible_) {
    // If parent object is set, visibility is determined by parent
    return parent_object_->IsVisible();
  }
  return visible_;
}

Eigen::Vector3d ObjectBase::GetGlobalPos() {
  UpdateMatrix();
  return global_mtx_.translation();
}

Eigen::Affine3d ObjectBase::GetGlobalMatrix() const { return global_mtx_; }

// NOLINTNEXTLINE
void ObjectBase::UpdateMatrix() {
  if (local_mtx_changed_) {
    // EigenではAffine3dを使って平行移動・回転・スケールを順番に合成
    Eigen::Affine3d translation =
        Eigen::Affine3d(Eigen::Translation3d(params_.pos));
    // NOLINTNEXTLINE
    Eigen::Affine3d rotation = Eigen::Affine3d(params_.quat);
    Eigen::Affine3d scale = Eigen::Affine3d(Eigen::Scaling(params_.scale));

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

void ObjectBase::RegisterParentObject(ObjectBase* obj) { parent_object_ = obj; }

}  // namespace livision