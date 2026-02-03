#include "livision/object/Mesh.hpp"

#include <vector>

namespace livision {

void Mesh::SetFromSTL(const std::string& path) {
  mesh_ = std::make_shared<MeshData>(path);
}

void Mesh::SetMeshData(const std::vector<Eigen::Vector3f>& vertices,
                       const std::vector<uint32_t>& indices) {
  mesh_ = std::make_shared<MeshData>(vertices, indices);
}

void Mesh::SetMeshData(std::shared_ptr<MeshData> mesh_data) {
  mesh_ = std::move(mesh_data);
}
}  // namespace livision
