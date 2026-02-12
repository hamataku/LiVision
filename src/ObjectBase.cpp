#include "livision/ObjectBase.hpp"

#include "livision/Renderer.hpp"

namespace livision {
void ObjectBase::OnDraw(Renderer& renderer) {
  if (mesh_buf_)
    renderer.Submit(*mesh_buf_, global_mtx_, params_.color, params_.texture,
                    params_.wire_color);
}

void ObjectBase::Init() {
  if (!is_initialized_) {
    OnInit();
  }
  is_initialized_ = true;
}

void ObjectBase::DeInit() {
  if (!is_initialized_) {
    return;
  }
  OnDeInit();
  is_initialized_ = false;
  mesh_buf_.reset();
}

void ObjectBase::UpdateMatrix(const Eigen::Affine3d& parent_mtx) {
  if (local_mtx_changed_) {
    Eigen::Affine3d translation(Eigen::Translation3d(params_.pos));
    Eigen::Affine3d rotation(params_.quat);
    Eigen::Affine3d scale(Eigen::Scaling(params_.scale));

    local_mtx_ = translation * rotation * scale;
    local_mtx_changed_ = false;
  }
  global_mtx_ = parent_mtx * local_mtx_;
}

ObjectBase* ObjectBase::SetParams(const Params& params) {
  params_ = params;
  name_ = params_.name;
  local_mtx_changed_ = true;
  return this;
}

ObjectBase* ObjectBase::SetPos(const Eigen::Vector3d& pos) {
  params_.pos = pos;
  local_mtx_changed_ = true;
  return this;
}
ObjectBase* ObjectBase::SetPos(double x, double y, double z) {
  params_.pos = Eigen::Vector3d(x, y, z);
  local_mtx_changed_ = true;
  return this;
}
ObjectBase* ObjectBase::SetScale(const Eigen::Vector3d& scale) {
  params_.scale = scale;
  local_mtx_changed_ = true;
  return this;
}
ObjectBase* ObjectBase::SetScale(double x, double y, double z) {
  params_.scale = Eigen::Vector3d(x, y, z);
  local_mtx_changed_ = true;
  return this;
}
ObjectBase* ObjectBase::SetQuatRotation(const Eigen::Quaterniond& q) {
  params_.quat = q;
  local_mtx_changed_ = true;
  return this;
}
ObjectBase* ObjectBase::SetDegRotation(const Eigen::Vector3d& euler_deg) {
  Eigen::Vector3d euler_rad = euler_deg * M_PI / 180.0;
  return SetRadRotation(euler_rad);
}
ObjectBase* ObjectBase::SetRadRotation(const Eigen::Vector3d& euler_rad) {
  Eigen::Quaterniond q =
      Eigen::AngleAxisd(euler_rad[2], Eigen::Vector3d::UnitZ()) *
      Eigen::AngleAxisd(euler_rad[1], Eigen::Vector3d::UnitY()) *
      Eigen::AngleAxisd(euler_rad[0], Eigen::Vector3d::UnitX());
  return SetQuatRotation(q);
}
ObjectBase* ObjectBase::SetVisible(bool visible) {
  visible_ = visible;
  return this;
}
ObjectBase* ObjectBase::SetColor(const Color& color) {
  params_.color = color;
  return this;
}
ObjectBase* ObjectBase::SetTexture(const std::string& texture) {
  params_.texture = texture;
  return this;
}
ObjectBase* ObjectBase::ClearTexture() {
  params_.texture.clear();
  return this;
}
ObjectBase* ObjectBase::SetWireColor(const Color& color) {
  params_.wire_color = color;
  return this;
}
ObjectBase* ObjectBase::SetName(const std::string& name) {
  name_ = name;
  params_.name = name;
  return this;
}

ObjectBase* ObjectBase::SetGlobalMatrix(const Eigen::Affine3d& mtx) {
  global_mtx_ = mtx;
  return this;
}

// NOLINTNEXTLINE
bool ObjectBase::IsVisible() const {
  if (parent_object_ && visible_) {
    // If parent object is set, visibility is determined by parent
    return parent_object_->IsVisible();
  }
  return visible_;
}

Eigen::Vector3d ObjectBase::GetGlobalPos() { return global_mtx_.translation(); }

Eigen::Affine3d ObjectBase::GetGlobalMatrix() const { return global_mtx_; }

const std::string& ObjectBase::GetName() const { return name_; }

void ObjectBase::RegisterParentObject(ObjectBase* obj) { parent_object_ = obj; }

}  // namespace livision
