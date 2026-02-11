#pragma once
#include <memory>
#include <vector>

#include "livision/Vertex.hpp"

namespace livision {

namespace internal {
struct MeshBufferAccess;
}  // namespace internal

/**
 * @brief GPU mesh buffers for vertices and indices.
 */
class MeshBuffer {
 public:
  /**
   * @brief Construct from vertices and indices.
   */
  MeshBuffer(std::vector<Vertex> vertices, std::vector<uint32_t> indices,
             bool has_uv = false);
  /**
   * @brief Destroy the mesh buffer.
   */
  ~MeshBuffer();

  /**
   * @brief Create vertex buffer on GPU.
   */
  void CreateVertex();
  /**
   * @brief Create index buffer on GPU.
   */
  void CreateIndex();
  /**
   * @brief Create wireframe index buffer on GPU.
   */
  void CreateWireIndex();

  /**
   * @brief Destroy GPU resources.
   */
  void Destroy();

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;

  friend struct internal::MeshBufferAccess;
};

}  // namespace livision
