#include "livision/MeshData.hpp"

#include <bgfx/bgfx.h>
#include <bgfx/platform.h>

#include <glm/glm.hpp>
#include <vector>

namespace livision {

struct MeshData::Impl {
  bgfx::VertexBufferHandle vbh = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh = BGFX_INVALID_HANDLE;
  uint32_t index_count = 0;
};

MeshData::MeshData(std::vector<Eigen::Vector3f> vertices,
                   std::vector<uint32_t> indices)
    : pimpl_(std::make_unique<Impl>()) {
  bgfx::VertexLayout vec3_vlayout;
  vec3_vlayout.begin()
      .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
      .end();

  std::vector<glm::vec3> glm_vertices(vertices.size());
  for (size_t i = 0; i < vertices.size(); i++)
    glm_vertices[i] = glm::vec3(vertices[i][0], vertices[i][1], vertices[i][2]);

  pimpl_->vbh = bgfx::createVertexBuffer(
      bgfx::makeRef(glm_vertices.data(),
                    glm_vertices.size() * sizeof(glm::vec3)),
      vec3_vlayout);

  pimpl_->ibh = bgfx::createIndexBuffer(
      bgfx::makeRef(indices.data(), indices.size() * sizeof(uint32_t)),
      BGFX_BUFFER_INDEX32);

  pimpl_->index_count = static_cast<uint32_t>(indices.size());
}

}  // namespace livision
