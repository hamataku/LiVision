#include "livision/MeshBuffer.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <utility>
#include <vector>

#include "livision/internal/mesh_buffer_access.hpp"
#include "livision/internal/stl_parser.hpp"

namespace livision {

struct MeshBuffer::Impl {
  bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  uint32_t index_count = 0;
  bool is_created = false;
};

MeshBuffer::MeshBuffer(std::vector<Vertex> vertices,
                       std::vector<uint32_t> indices)
    : pimpl_(std::make_unique<Impl>()) {
  pimpl_->vertices = std::move(vertices);
  pimpl_->indices = std::move(indices);
}

MeshBuffer::MeshBuffer(const std::string& stl_path)
    : pimpl_(std::make_unique<Impl>()) {
  stl_parser::ParseSTLFile(stl_path, pimpl_->vertices, pimpl_->indices);
}

MeshBuffer::~MeshBuffer() { Destroy(); }

void MeshBuffer::Destroy() {
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

void MeshBuffer::Create() {
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

namespace internal {

bgfx::VertexBufferHandle MeshBufferAccess::VertexBuffer(
    const MeshBuffer& mesh) {
  return mesh.pimpl_->vbh;
}

bgfx::IndexBufferHandle MeshBufferAccess::IndexBuffer(const MeshBuffer& mesh) {
  return mesh.pimpl_->ibh;
}

uint32_t MeshBufferAccess::IndexCount(const MeshBuffer& mesh) {
  return mesh.pimpl_->index_count;
}

}  // namespace internal

}  // namespace livision
