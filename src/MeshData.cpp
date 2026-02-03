#include "livision/MeshData.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <glm/glm.hpp>
#include <utility>
#include <vector>

#include "livision/internal/mesh_data_access.hpp"

namespace livision {

struct MeshData::Impl {
  bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

  std::vector<glm::vec3> glm_vertices;
  std::vector<uint32_t> indices;
  uint32_t index_count = 0;
};

MeshData::MeshData(std::vector<Eigen::Vector3f> vertices_,
                   std::vector<uint32_t> indices_)
    : pimpl_(std::make_unique<Impl>()) {
  pimpl_->indices = std::move(indices_);

  bgfx::VertexLayout vec3_vlayout;
  vec3_vlayout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  pimpl_->glm_vertices.resize(vertices_.size());
  for (size_t i = 0; i < vertices_.size(); i++) {
    pimpl_->glm_vertices[i] =
        glm::vec3(vertices_[i][0], vertices_[i][1], vertices_[i][2]);
  }

  pimpl_->vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(pimpl_->glm_vertices.data(),
                    pimpl_->glm_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);

  pimpl_->ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(pimpl_->indices.data(),
                    pimpl_->indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);

  pimpl_->index_count = static_cast<uint32_t>(pimpl_->indices.size());
}

MeshData::~MeshData() = default;

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
