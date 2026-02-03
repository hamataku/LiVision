#pragma once
#include <Eigen/Core>
#include <memory>
#include <vector>

namespace livision {

class MeshData {
 public:
  MeshData(std::vector<Eigen::Vector3f> vertices,
           std::vector<uint32_t> indices);

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  friend class Renderer;
};

}  // namespace livision