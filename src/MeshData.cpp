#include "livision/MeshData.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <utility>
#include <vector>

#include "livision/internal/mesh_data_access.hpp"
#include "livision/internal/stl_parser.hpp"

namespace livision {

struct MeshData::Impl {
  bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  uint32_t index_count = 0;
  bool is_created = false;
};

MeshData::MeshData(std::vector<Vertex> vertices, std::vector<uint32_t> indices)
    : pimpl_(std::make_unique<Impl>()) {
  pimpl_->vertices = std::move(vertices);
  pimpl_->indices = std::move(indices);
}

MeshData::MeshData(const std::string& stl_path)
    : pimpl_(std::make_unique<Impl>()) {
  stl_parser::ParseSTLFile(stl_path, pimpl_->vertices, pimpl_->indices);
}

MeshData::~MeshData() { DestroyBuffer(); }

void MeshData::DestroyBuffer() {
  if (!pimpl_->is_created) {
    return;
  }
  if (bgfx::isValid(pimpl_->vbh)) {
    bgfx::destroy(pimpl_->vbh);
    pimpl_->vbh = BGFX_INVALID_HANDLE;
  }
  if (bgfx::isValid(pimpl_->ibh)) {
    bgfx::destroy(pimpl_->ibh);
    pimpl_->ibh = BGFX_INVALID_HANDLE;
  }
  pimpl_->is_created = false;
}

void MeshData::CreateBuffer() {
  if (pimpl_->is_created) {
    return;
  }
  bgfx::VertexLayout vec3_vlayout;
  vec3_vlayout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  pimpl_->vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(pimpl_->vertices.data(),
                    pimpl_->vertices.size() * sizeof(Vertex)),
      vec3_vlayout);

  pimpl_->ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(pimpl_->indices.data(),
                    pimpl_->indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);

  pimpl_->index_count = static_cast<uint32_t>(pimpl_->indices.size());
  pimpl_->is_created = true;
}

std::vector<Vertex> MeshData::GetVertices() { return pimpl_->vertices; }
std::vector<uint32_t> MeshData::GetIndices() { return pimpl_->indices; }

namespace internal {

bgfx::VertexBufferHandle MeshDataAccess::VertexBuffer(const MeshData& mesh) {
  return mesh.pimpl_->vbh;
}

bgfx::IndexBufferHandle MeshDataAccess::IndexBuffer(const MeshData& mesh) {
  return mesh.pimpl_->ibh;
}

uint32_t MeshDataAccess::IndexCount(const MeshData& mesh) {
  return mesh.pimpl_->index_count;
}

}  // namespace internal

}  // namespace livision
