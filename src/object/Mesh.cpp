#include "livision/object/Mesh.hpp"

#include <vector>

namespace livision {

void Mesh::SetMeshData(const std::vector<Vertex>& vertices,
                       const std::vector<uint32_t>& indices, bool has_uv) {
  mesh_buf_ = std::make_shared<MeshBuffer>(vertices, indices, has_uv);
}

void Mesh::SetMeshBuffer(std::shared_ptr<MeshBuffer> mesh_buffer) {
  mesh_buf_ = std::move(mesh_buffer);
}

}  // namespace livision
