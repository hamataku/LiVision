#include "livision/object/Mesh.hpp"

#include <vector>

#include "livision/internal/mesh_buffer_manager.hpp"

namespace livision {

void Mesh::SetMeshData(const std::vector<Vertex>& vertices,
                       const std::vector<uint32_t>& indices, bool has_uv) {
  mesh_buf_ = internal::MeshBufferManager::CreateTracked(vertices, indices, has_uv);
}

void Mesh::SetMeshBuffer(std::shared_ptr<MeshBuffer> mesh_buffer) {
  internal::MeshBufferManager::Register(mesh_buffer);
  mesh_buf_ = std::move(mesh_buffer);
}

}  // namespace livision
