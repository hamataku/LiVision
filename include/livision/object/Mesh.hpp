#pragma once

#include <memory>

#include "livision/MeshBuffer.hpp"
#include "livision/ObjectBase.hpp"

namespace livision {

class Mesh : public ObjectBase {
 public:
  explicit Mesh(Params params = Params()) : ObjectBase(std::move(params)) {}
  explicit Mesh(const std::string& path, Params params = Params())
      : ObjectBase(std::move(params)) {
    SetFromSTL(path);
  }
  Mesh(const std::vector<Vertex>& vertices,
       const std::vector<uint32_t>& indices, Params params = Params())
      : ObjectBase(std::move(params)) {
    SetMeshData(vertices, indices);
  }
  explicit Mesh(std::shared_ptr<MeshBuffer> mesh_buffer,
                Params params = Params())
      : ObjectBase(std::move(params)) {
    SetMeshBuffer(std::move(mesh_buffer));
  }

  void SetFromSTL(const std::string& path);
  void SetMeshData(const std::vector<Vertex>& vertices,
                   const std::vector<uint32_t>& indices);
  void SetMeshBuffer(std::shared_ptr<MeshBuffer> mesh_buffer);
};
}  // namespace livision
