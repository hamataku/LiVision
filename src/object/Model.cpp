#include "livision/object/Model.hpp"

#include <bit>
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <unordered_map>

#include "livision/Log.hpp"
#include "livision/ObjectBase.hpp"
#include "livision/internal/mesh_buffer_manager.hpp"
#include "livision/internal/sdf_loader.hpp"
#include "livision/object/Mesh.hpp"
#include "livision/object/primitives.hpp"

namespace livision {

namespace {
namespace fs = std::filesystem;

struct ModelCpuCacheEntrySdf {
  std::weak_ptr<const internal::sdf_loader::SdfNode> data;
};

struct ModelCpuCacheEntryMesh {
  std::weak_ptr<const std::vector<internal::sdf_loader::MeshPart>> data;
};

std::unordered_map<std::string, ModelCpuCacheEntrySdf>& SdfSceneCache() {
  static std::unordered_map<std::string, ModelCpuCacheEntrySdf> cache;
  return cache;
}

std::unordered_map<std::string, ModelCpuCacheEntryMesh>& MeshPartsCache() {
  static std::unordered_map<std::string, ModelCpuCacheEntryMesh> cache;
  return cache;
}

std::string NormalizeCacheKey(const std::string& path) {
  std::error_code ec;
  const fs::path canonical = fs::weakly_canonical(fs::path(path), ec);
  if (!ec) {
    return canonical.string();
  }
  return path;
}

void HashCombine(std::size_t& seed, std::size_t value) {
  seed ^= value + 0x9e3779b97f4a7c15ULL + (seed << 6U) + (seed >> 2U);
}

std::size_t HashVertex(const Vertex& v) {
  std::size_t seed = 0;
  HashCombine(seed, std::hash<uint32_t>{}(std::bit_cast<uint32_t>(v.x)));
  HashCombine(seed, std::hash<uint32_t>{}(std::bit_cast<uint32_t>(v.y)));
  HashCombine(seed, std::hash<uint32_t>{}(std::bit_cast<uint32_t>(v.z)));
  HashCombine(seed, std::hash<uint32_t>{}(std::bit_cast<uint32_t>(v.u)));
  HashCombine(seed, std::hash<uint32_t>{}(std::bit_cast<uint32_t>(v.v)));
  return seed;
}

std::string BuildMeshKey(const std::string& tag,
                         const std::vector<Vertex>& vertices,
                         const std::vector<uint32_t>& indices, bool has_uv) {
  std::size_t seed = std::hash<std::string>{}(tag);
  HashCombine(seed, vertices.size());
  HashCombine(seed, indices.size());
  HashCombine(seed, static_cast<std::size_t>(has_uv));
  for (const auto& v : vertices) {
    HashCombine(seed, HashVertex(v));
  }
  for (uint32_t idx : indices) {
    HashCombine(seed, std::hash<uint32_t>{}(idx));
  }
  return tag + ":" + std::to_string(seed);
}

std::shared_ptr<const internal::sdf_loader::SdfNode> AcquireSdfScene(
    const std::string& path, bool force_reload, std::string* error) {
  auto& cache = SdfSceneCache();
  const std::string key = NormalizeCacheKey(path);

  if (!force_reload) {
    auto it = cache.find(key);
    if (it != cache.end()) {
      if (auto cached = it->second.data.lock()) {
        return cached;
      }
    }
  }

  auto scene = std::make_shared<internal::sdf_loader::SdfNode>();
  if (!internal::sdf_loader::LoadSdfScene(path, *scene, error)) {
    return {};
  }
  cache[key].data = scene;
  return scene;
}

std::shared_ptr<const std::vector<internal::sdf_loader::MeshPart>>
AcquireMeshParts(const std::string& path, bool force_reload, std::string* error) {
  auto& cache = MeshPartsCache();
  const std::string key = NormalizeCacheKey(path);

  if (!force_reload) {
    auto it = cache.find(key);
    if (it != cache.end()) {
      if (auto cached = it->second.data.lock()) {
        return cached;
      }
    }
  }

  auto parts = std::make_shared<std::vector<internal::sdf_loader::MeshPart>>();
  if (!internal::sdf_loader::LoadMeshFileParts(path, *parts, error)) {
    return {};
  }
  cache[key].data = parts;
  return parts;
}

std::string ToLower(std::string value) {
  std::ranges::transform(value, value.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
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

Model* Model::SetFromFile(const std::string& path, LoadOptions options) {
  ClearObjects();
  GetMeshBuffer().reset();
  SetName("");

  std::string error;
  if (HasExtension(path, "sdf")) {
    auto scene = AcquireSdfScene(path, options.force_reload, &error);
    if (!scene) {
      LogMessage(LogLevel::Error, "Failed to load SDF: ", path,
                 error.empty() ? "" : "\n", error);
      return this;
    }
    if (scene->tag == "sdf" && scene->children.size() == 1U) {
      BuildFromNode(scene->children.front(), false);
    } else {
      BuildFromNode(*scene, false);
    }
    return this;
  }

  auto mesh_parts = AcquireMeshParts(path, options.force_reload, &error);
  if (!mesh_parts) {
    LogMessage(LogLevel::Error, "Failed to load mesh file: ", path,
               error.empty() ? "" : "\n", error);
    return this;
  }

  std::size_t mesh_index = 0;
  for (const auto& part : *mesh_parts) {
    auto mesh = std::make_shared<Mesh>();
    mesh->SetName("mesh#" + std::to_string(mesh_index++));
    const std::string mesh_key =
        BuildMeshKey("model:file_mesh", part.vertices, part.indices, part.has_uv);
    auto mesh_buf = internal::MeshBufferManager::AcquireShared(
        mesh_key, [&part]() {
          return std::make_shared<MeshBuffer>(part.vertices, part.indices,
                                              part.has_uv);
        });
    if (mesh_buf) {
      mesh->SetMeshBuffer(std::move(mesh_buf));
    } else {
      mesh->SetMeshData(part.vertices, part.indices, part.has_uv);
    }
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
    AddOwned(mesh);
  }
  return this;
}

void Model::AddOwned(std::shared_ptr<ObjectBase> child) {
  AddObject(std::move(child));
}

void Model::BuildFromNode(const internal::sdf_loader::SdfNode& node,
                          bool apply_self_transform) {
  SetName(node.effective_name);

  if (apply_self_transform) {
    SetPos(node.pos);
    SetQuatRotation(node.rot);
    SetScale(node.scale);
  }

  if (node.HasMesh()) {
    auto mesh = std::make_shared<Mesh>();
    mesh->SetName("mesh#0");
    const std::string mesh_key =
        BuildMeshKey("model:sdf_node", node.vertices, node.indices, node.has_uv);
    auto mesh_buf = internal::MeshBufferManager::AcquireShared(
        mesh_key, [&node]() {
          return std::make_shared<MeshBuffer>(node.vertices, node.indices,
                                              node.has_uv);
        });
    if (mesh_buf) {
      mesh->SetMeshBuffer(std::move(mesh_buf));
    } else {
      mesh->SetMeshData(node.vertices, node.indices, node.has_uv);
    }
    mesh->SetColor(node.color);
    if (!node.texture.empty()) {
      mesh->SetTexture(node.texture);
    }
    AddOwned(mesh);
  } else if (node.HasPrimitive()) {
    auto buffer = BufferFromPrimitive(node.primitive);
    if (buffer) {
      auto mesh = std::make_shared<Mesh>();
      mesh->SetName("mesh#0");
      mesh->SetMeshBuffer(std::move(buffer));
      mesh->SetColor(node.color);
      AddOwned(mesh);
    }
  }

  for (const auto& child : node.children) {
    auto child_model = std::make_shared<Model>();
    child_model->BuildFromNode(child, true);
    AddOwned(child_model);
  }
}

}  // namespace livision
