#pragma once
#include <Eigen/Core>
#include <memory>
#include <vector>

namespace livision {

namespace internal {
struct MeshDataAccess;
}  // namespace internal

class MeshData {
 public:
  MeshData(std::vector<Eigen::Vector3f> vertices,
           std::vector<uint32_t> indices);
  explicit MeshData(const std::string &stl_path);
  ~MeshData();

  void Destroy();

  std::vector<Eigen::Vector3f> GetVertices();
  std::vector<uint32_t> GetIndices();

 private:
  void CreateBuffers(std::vector<Eigen::Vector3f> &vertices,
                     std::vector<uint32_t> &indices);
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  friend struct internal::MeshDataAccess;
};

}  // namespace livision