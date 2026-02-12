#include "livision/Container.hpp"

namespace livision {

void Container::OnInit() {
  for (auto& object : objects_) {
    object->Init();
  }
}

void Container::OnDeInit() {
  for (auto& object : objects_) {
    object->DeInit();
  }
}

void Container::OnDraw(Renderer& renderer) {
  for (const auto& object : objects_) {
    if (object->IsVisible()) object->OnDraw(renderer);
  }
}

void Container::UpdateMatrix(const Eigen::Affine3d& parent_mtx) {
  if (local_mtx_changed_) {
    Eigen::Affine3d translation(Eigen::Translation3d(params_.pos));
    Eigen::Affine3d rotation(params_.quat);
    Eigen::Affine3d scale(Eigen::Scaling(params_.scale));

    local_mtx_ = translation * rotation * scale;
    local_mtx_changed_ = false;
  }
  global_mtx_ = parent_mtx * local_mtx_;
  for (const auto& object : objects_) {
    object->UpdateMatrix(global_mtx_);
  }
}

Container* Container::AddObject(std::shared_ptr<ObjectBase> object) {
  if (!object) {
    return this;
  }
  object->RegisterParentObject(this);
  if (is_initialized_) {
    object->Init();
  }
  objects_.push_back(object);
  return this;
}

std::vector<std::shared_ptr<ObjectBase>>& Container::GetObjects() {
  return objects_;
}

const std::vector<std::shared_ptr<ObjectBase>>& Container::GetObjects() const {
  return objects_;
}

void Container::ClearObjects() { objects_.clear(); }

}  // namespace livision
