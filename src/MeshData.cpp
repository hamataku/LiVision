#include "livision/MeshData.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <glm/glm.hpp>
#include <utility>
#include <vector>

#include "livision/internal/mesh_data_access.hpp"
#include "livision/internal/stl_parser.hpp"

namespace livision {

struct MeshData::Impl {
  bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;

  std::vector<glm::vec3> glm_vertices;
  std::vector<uint32_t> indices;
  uint32_t index_count = 0;
  bool is_destroyed = false;
};

MeshData::MeshData(std::vector<Eigen::Vector3f> vertices,
                   std::vector<uint32_t> indices)
    : pimpl_(std::make_unique<Impl>()) {
  CreateBuffers(vertices, indices);
}

MeshData::MeshData(const std::string& stl_path)
    : pimpl_(std::make_unique<Impl>()) {
  std::vector<Eigen::Vector3f> vertices;
  std::vector<uint32_t> indices;
  stl_parser::ParseSTLFile(stl_path, vertices, indices);
  CreateBuffers(vertices, indices);
}

MeshData::~MeshData() { Destroy(); }

void MeshData::Destroy() {
  if (pimpl_->is_destroyed) {
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
  pimpl_->is_destroyed = true;
}

void MeshData::CreateBuffers(std::vector<Eigen::Vector3f>& vertices,
                             std::vector<uint32_t>& indices) {
  pimpl_->indices = std::move(indices);

  bgfx::VertexLayout vec3_vlayout;
  vec3_vlayout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  pimpl_->glm_vertices.resize(vertices.size());
  for (size_t i = 0; i < vertices.size(); i++) {
    pimpl_->glm_vertices[i] =
        glm::vec3(vertices[i][0], vertices[i][1], vertices[i][2]);
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

std::vector<Eigen::Vector3f> MeshData::GetVertices() {
  std::vector<Eigen::Vector3f> vertices;
  vertices.reserve(pimpl_->glm_vertices.size());
  for (const auto& v : pimpl_->glm_vertices) {
    vertices.emplace_back(v.x, v.y, v.z);
  }
  return vertices;
}

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
