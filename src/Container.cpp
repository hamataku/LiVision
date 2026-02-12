#include "livision/Container.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "livision/Log.hpp"

namespace livision {
namespace {
std::string NodeClassName(const ObjectBase& object) {
  return dynamic_cast<const Container*>(&object) ? "Container" : "ObjectBase";
}

void AppendTreeNode(std::ostringstream& oss, const ObjectBase& object,
                    std::size_t depth, const std::string& path) {
  const auto* as_container = dynamic_cast<const Container*>(&object);
  oss << std::string(depth * 2U, ' ') << "- " << NodeClassName(object)
      << " name=\"" << object.GetName() << "\" path=\"" << path << "\"\n";

  if (!as_container) {
    return;
  }

  for (const auto& child : as_container->GetObjects()) {
    if (!child) {
      continue;
    }
    std::string child_path = path;
    if (!child->GetName().empty()) {
      if (!child_path.empty()) {
        child_path += "/";
      }
      child_path += child->GetName();
    }
    AppendTreeNode(oss, *child, depth + 1U, child_path);
  }
}

std::vector<std::string> SplitPath(const std::string& path) {
  std::vector<std::string> tokens;
  std::string current;
  for (const char c : path) {
    if (c == '/') {
      if (!current.empty()) {
        tokens.push_back(current);
        current.clear();
      }
      continue;
    }
    current.push_back(c);
  }
  if (!current.empty()) {
    tokens.push_back(current);
  }
  return tokens;
}
}  // namespace

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

std::shared_ptr<ObjectBase> Container::GetChild(const std::string& name) const {
  for (const auto& object : objects_) {
    if (object && object->GetName() == name) {
      return object;
    }
  }
  return {};
}

std::vector<std::shared_ptr<ObjectBase>> Container::GetChildren(
    const std::string& name) const {
  std::vector<std::shared_ptr<ObjectBase>> found;
  for (const auto& object : objects_) {
    if (object && object->GetName() == name) {
      found.push_back(object);
    }
  }
  return found;
}

std::shared_ptr<Container> Container::GetChildContainer(
    const std::string& name) const {
  return std::dynamic_pointer_cast<Container>(GetChild(name));
}

std::shared_ptr<ObjectBase> Container::GetByPath(
    const std::string& path) const {
  const auto parts = SplitPath(path);
  if (parts.empty()) {
    return {};
  }

  const Container* current_container = this;
  std::shared_ptr<ObjectBase> current_object;

  for (std::size_t i = 0; i < parts.size(); ++i) {
    if (!current_container) {
      return {};
    }
    current_object = current_container->GetChild(parts[i]);
    if (!current_object) {
      return {};
    }
    if (i + 1U < parts.size()) {
      current_container = dynamic_cast<const Container*>(current_object.get());
    }
  }
  return current_object;
}

std::shared_ptr<Container> Container::GetContainerByPath(
    const std::string& path) const {
  return std::dynamic_pointer_cast<Container>(GetByPath(path));
}

std::string Container::DumpTree() const {
  std::ostringstream oss;
  AppendTreeNode(oss, *this, 0, "");
  return oss.str();
}

void Container::PrintTree(LogLevel level) const {
  std::cout << DumpTree() << std::endl;
}

}  // namespace livision
