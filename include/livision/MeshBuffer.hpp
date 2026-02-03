#pragma once
#include <memory>
#include <vector>

#include "livision/Vertex.hpp"

namespace livision {

namespace internal {
struct MeshBufferAccess;
}  // namespace internal

class MeshBuffer {
 public:
  MeshBuffer(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
  explicit MeshBuffer(const std::string &stl_path);
  ~MeshBuffer();

  void Create();
  void Destroy();

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  friend struct internal::MeshBufferAccess;
};

}  // namespace livision