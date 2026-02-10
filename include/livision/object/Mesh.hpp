#pragma once

#include <memory>

#include "livision/MeshBuffer.hpp"
#include "livision/ObjectBase.hpp"

namespace livision {

/**
 * @brief Renderable mesh object.
 * @ingroup object
 */
class Mesh : public ObjectBase {
 public:
  /**
   * @brief Construct with optional parameters.
   */
  explicit Mesh(Params params = Params()) : ObjectBase(std::move(params)) {}
  /**
   * @brief Construct from vertex and index arrays.
   */
  Mesh(const std::vector<Vertex>& vertices,
       const std::vector<uint32_t>& indices, Params params = Params())
      : ObjectBase(std::move(params)) {
    SetMeshData(vertices, indices);
  }
  /**
   * @brief Construct from an existing mesh buffer.
   */
  explicit Mesh(std::shared_ptr<MeshBuffer> mesh_buffer,
                Params params = Params())
      : ObjectBase(std::move(params)) {
    SetMeshBuffer(std::move(mesh_buffer));
  }

  /**
   * @brief Set mesh data from vertices and indices.
   */
  void SetMeshData(const std::vector<Vertex>& vertices,
                   const std::vector<uint32_t>& indices);
  /**
   * @brief Set the mesh buffer directly.
   */
  void SetMeshBuffer(std::shared_ptr<MeshBuffer> mesh_buffer);
};
}  // namespace livision
