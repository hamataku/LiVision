#include "livision/object/Model.hpp"

#include <algorithm>
#include <cctype>
#include <iostream>

#include "livision/internal/sdf_loader.hpp"
#include "livision/Log.hpp"
#include "livision/object/primitives.hpp"

namespace livision {

namespace {
std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

bool HasExtension(const std::string& path, const std::string& ext) {
  const auto dot = path.find_last_of('.');
  if (dot == std::string::npos) {
    return false;
  }
  return ToLower(path.substr(dot + 1)) == ext;
}

bool IsDefaultWhite(const Color& color) {
  return color.mode == Color::ColorMode::Fixed && color.base[0] == 1.0F &&
         color.base[1] == 1.0F && color.base[2] == 1.0F &&
         color.base[3] == 1.0F;
}

std::shared_ptr<MeshBuffer> BufferFromPrimitive(
    internal::sdf_loader::SdfNode::PrimitiveType type) {
  switch (type) {
    case internal::sdf_loader::SdfNode::PrimitiveType::Box: {
      Box prim;
      return prim.GetMeshBuffer();
    }
    case internal::sdf_loader::SdfNode::PrimitiveType::Sphere: {
      Sphere prim;
      return prim.GetMeshBuffer();
    }
    case internal::sdf_loader::SdfNode::PrimitiveType::Cylinder: {
      Cylinder prim;
      return prim.GetMeshBuffer();
    }
    case internal::sdf_loader::SdfNode::PrimitiveType::Cone: {
      Cone prim;
      return prim.GetMeshBuffer();
    }
    case internal::sdf_loader::SdfNode::PrimitiveType::Plane: {
      Plane prim;
      return prim.GetMeshBuffer();
    }
    case internal::sdf_loader::SdfNode::PrimitiveType::None:
      break;
  }
  return {};
}
}  // namespace

void Model::SetFromFile(const std::string& path) {
  ClearChildren();
  GetMeshBuffer().reset();

  std::string error;
  if (HasExtension(path, "sdf")) {
    internal::sdf_loader::SdfNode root;
    if (!internal::sdf_loader::LoadSdfScene(path, root, &error)) {
      LogMessage(LogLevel::Error, "Failed to load SDF: ", path,
                 error.empty() ? "" : "\n", error);
      return;
    }
    BuildFromNode(root);
    return;
  }

  std::vector<internal::sdf_loader::MeshPart> mesh_parts;
  if (!internal::sdf_loader::LoadMeshFileParts(path, mesh_parts, &error)) {
    LogMessage(LogLevel::Error, "Failed to load mesh file: ", path,
               error.empty() ? "" : "\n", error);
    return;
  }

  for (auto& part : mesh_parts) {
    auto mesh = std::make_unique<Mesh>();
    mesh->SetMeshData(part.vertices, part.indices, part.has_uv);
    // User-specified model color should override assimp material color
    // (e.g. rainbow_z in examples). Material color is used only when the model
    // color remains the default white.
    if (!IsDefaultWhite(params_.color)) {
      mesh->SetColor(params_.color);
    } else if (part.has_color) {
      mesh->SetColor(part.color);
    } else {
      mesh->SetColor(params_.color);
    }
    if (!part.texture_uri.empty()) {
      mesh->SetTexture(part.texture_uri);
    }
    mesh->SetWireColor(params_.wire_color);
    AddOwned(std::move(mesh));
  }
}

void Model::ClearChildren() {
  owned_children_.clear();
  ClearObjects();
}

void Model::AddOwned(std::unique_ptr<ObjectBase> child) {
  AddObject(child.get());
  owned_children_.push_back(std::move(child));
}

void Model::BuildFromNode(const internal::sdf_loader::SdfNode& node) {
  SetPos(node.pos);
  SetQuatRotation(node.rot);
  SetScale(node.scale);

  if (node.HasMesh()) {
    auto mesh = std::make_unique<Mesh>();
    mesh->SetMeshData(node.vertices, node.indices, node.has_uv);
    mesh->SetColor(node.color);
    if (!node.texture.empty()) {
      mesh->SetTexture(node.texture);
    }
    AddOwned(std::move(mesh));
  } else if (node.HasPrimitive()) {
    auto buffer = BufferFromPrimitive(node.primitive);
    if (buffer) {
      auto mesh = std::make_unique<Mesh>();
      mesh->SetMeshBuffer(std::move(buffer));
      mesh->SetColor(node.color);
      AddOwned(std::move(mesh));
    }
  }

  for (const auto& child : node.children) {
    auto child_model = std::make_unique<Model>();
    child_model->BuildFromNode(child);
    AddOwned(std::move(child_model));
  }
}

}  // namespace livision
