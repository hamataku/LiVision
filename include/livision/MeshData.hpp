#pragma once
#include <memory>
#include <vector>

#include "livision/Vertex.hpp"

namespace livision {

namespace internal {
struct MeshDataAccess;
}  // namespace internal

class MeshData {
 public:
  MeshData(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  explicit MeshData(const std::string &stl_path);
  ~MeshData();

  void CreateBuffer();
  void DestroyBuffer();

  std::vector<Vertex> GetVertices();
  std::vector<uint32_t> GetIndices();

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  friend struct internal::MeshDataAccess;
};

}  // namespace livision