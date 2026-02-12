#include "livision/internal/mesh_buffer_manager.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace livision::internal {

namespace {
struct ManagerState {
  bool bgfx_alive = false;
  std::unordered_map<std::string, std::weak_ptr<MeshBuffer>> shared_cache;
  std::vector<std::weak_ptr<MeshBuffer>> tracked;
};

ManagerState& State() {
  static ManagerState state;
  return state;
}

void PruneTracked(std::vector<std::weak_ptr<MeshBuffer>>& tracked) {
  tracked.erase(std::remove_if(tracked.begin(), tracked.end(),
                               [](const std::weak_ptr<MeshBuffer>& weak) {
                                 return weak.expired();
                               }),
                tracked.end());
}
}  // namespace

std::shared_ptr<MeshBuffer> MeshBufferManager::AcquireShared(
    const std::string& key, const MeshFactory& factory) {
  auto& state = State();
  auto it = state.shared_cache.find(key);
  if (it != state.shared_cache.end()) {
    if (auto existing = it->second.lock()) {
      Register(existing);
      return existing;
    }
  }

  if (!factory) {
    return {};
  }

  auto created = factory();
  if (!created) {
    return {};
  }
  state.shared_cache[key] = created;
  Register(created);
  return created;
}

std::shared_ptr<MeshBuffer> MeshBufferManager::CreateTracked(
    std::vector<Vertex> vertices, std::vector<uint32_t> indices, bool has_uv) {
  auto mesh =
      std::make_shared<MeshBuffer>(std::move(vertices), std::move(indices), has_uv);
  Register(mesh);
  return mesh;
}

void MeshBufferManager::Register(const std::shared_ptr<MeshBuffer>& mesh) {
  if (!mesh) {
    return;
  }
  auto& tracked = State().tracked;
  if ((tracked.size() % 64) == 0) {
    PruneTracked(tracked);
  }
  tracked.emplace_back(mesh);
}

void MeshBufferManager::DestroyAllBuffers() {
  auto& state = State();
  std::vector<std::shared_ptr<MeshBuffer>> alive_meshes;
  std::unordered_set<MeshBuffer*> seen;

  alive_meshes.reserve(state.tracked.size() + state.shared_cache.size());
  seen.reserve(state.tracked.size() + state.shared_cache.size());

  for (const auto& weak : state.tracked) {
    if (auto mesh = weak.lock()) {
      if (seen.insert(mesh.get()).second) {
        alive_meshes.emplace_back(std::move(mesh));
      }
    }
  }

  for (const auto& [_, weak] : state.shared_cache) {
    if (auto mesh = weak.lock()) {
      if (seen.insert(mesh.get()).second) {
        alive_meshes.emplace_back(std::move(mesh));
      }
    }
  }

  for (auto& mesh : alive_meshes) {
    mesh->Destroy();
  }

  state.shared_cache.clear();
  state.tracked.clear();
}

void MeshBufferManager::SetBgfxAlive(bool alive) { State().bgfx_alive = alive; }

bool MeshBufferManager::IsBgfxAlive() { return State().bgfx_alive; }

}  // namespace livision::internal
