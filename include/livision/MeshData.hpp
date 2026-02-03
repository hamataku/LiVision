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
  MeshData(std::vector<Eigen::Vector3f> vertices_,
           std::vector<uint32_t> indices_);
  ~MeshData();

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  friend struct internal::MeshDataAccess;
  friend class Renderer;
};

}  // namespace livision