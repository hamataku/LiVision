#include "livision/MeshBuffer.hpp"

#include <unordered_set>

#include "livision/internal/mesh_buffer_access.hpp"
#include "livision/internal/stl_parser.hpp"

namespace livision {

struct MeshBuffer::Impl {
  bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle wire_ibh = BGFX_INVALID_HANDLE;

  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
  std::vector<uint32_t> wire_indices;
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
  if (bgfx::isValid(pimpl_->vbh)) {
    bgfx::destroy(pimpl_->vbh);
    pimpl_->vbh = BGFX_INVALID_HANDLE;
  }
  if (bgfx::isValid(pimpl_->ibh)) {
    bgfx::destroy(pimpl_->ibh);
    pimpl_->ibh = BGFX_INVALID_HANDLE;
  }
  if (bgfx::isValid(pimpl_->wire_ibh)) {
    bgfx::destroy(pimpl_->wire_ibh);
    pimpl_->wire_ibh = BGFX_INVALID_HANDLE;
  }
}

void MeshBuffer::CreateVertex() {
  if (bgfx::isValid(pimpl_->vbh)) {
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
}

void MeshBuffer::CreateIndex() {
  if (bgfx::isValid(pimpl_->ibh)) {
    return;
  }

  pimpl_->ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(pimpl_->indices.data(),
                    pimpl_->indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);
}

void MeshBuffer::CreateWireIndex() {
  if (bgfx::isValid(pimpl_->wire_ibh)) {
    return;
  }

  // Build wireframe index buffer (unique edges)
  pimpl_->wire_indices.reserve(pimpl_->indices.size() * 2);
  std::unordered_set<uint64_t> seen_edges;
  seen_edges.reserve(pimpl_->indices.size());

  auto add_edge = [&](uint32_t a, uint32_t b) {
    if (a == b) return;
    uint32_t lo = (a < b) ? a : b;
    uint32_t hi = (a < b) ? b : a;
    uint64_t key = (static_cast<uint64_t>(lo) << 32) | hi;
    if (seen_edges.insert(key).second) {
      pimpl_->wire_indices.push_back(lo);
      pimpl_->wire_indices.push_back(hi);
    }
  };

  for (size_t i = 0; i + 2 < pimpl_->indices.size(); i += 3) {
    uint32_t i0 = pimpl_->indices[i];
    uint32_t i1 = pimpl_->indices[i + 1];
    uint32_t i2 = pimpl_->indices[i + 2];
    add_edge(i0, i1);
    add_edge(i1, i2);
    add_edge(i2, i0);
  }

  pimpl_->wire_ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(pimpl_->wire_indices.data(),
                    pimpl_->wire_indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);
}

namespace internal {

bgfx::VertexBufferHandle MeshBufferAccess::VertexBuffer(MeshBuffer& mesh) {
  mesh.CreateVertex();
  return mesh.pimpl_->vbh;
}

bgfx::IndexBufferHandle MeshBufferAccess::IndexBuffer(MeshBuffer& mesh) {
  mesh.CreateIndex();
  return mesh.pimpl_->ibh;
}

bgfx::IndexBufferHandle MeshBufferAccess::WireIndexBuffer(MeshBuffer& mesh) {
  mesh.CreateWireIndex();
  return mesh.pimpl_->wire_ibh;
}

}  // namespace internal

}  // namespace livision
