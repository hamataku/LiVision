#pragma once

#include <unordered_set>

#include "LiVision/MeshParser.hpp"
#include "LiVision/ObjectBase.hpp"

namespace livision {

class WireFrame : public ObjectBase {
 public:
  explicit WireFrame(const std::string& path) {
    mesh_parser::ParseMeshFile(path, vertices_, indices_);
  }

  void InitImpl() override {
    vbh_ = bgfx::createVertexBuffer(
        bgfx::makeRef(vertices_.data(), vertices_.size() * sizeof(glm::vec3)),
        utils::vec3_vlayout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(indices_.data(), indices_.size() * sizeof(uint32_t)),
        BGFX_BUFFER_INDEX32);

    // Build wireframe index buffer (unique edges)
    line_indices_.clear();
    line_indices_.reserve(indices_.size() * 2);
    std::unordered_set<uint64_t> seen_edges;
    seen_edges.reserve(indices_.size());

    auto add_edge = [&](uint32_t a, uint32_t b) {
      if (a == b) return;
      uint32_t lo = (a < b) ? a : b;
      uint32_t hi = (a < b) ? b : a;
      uint64_t key = (static_cast<uint64_t>(lo) << 32) | hi;
      if (seen_edges.insert(key).second) {
        line_indices_.push_back(lo);
        line_indices_.push_back(hi);
      }
    };

    for (size_t i = 0; i + 2 < indices_.size(); i += 3) {
      uint32_t i0 = indices_[i];
      uint32_t i1 = indices_[i + 1];
      uint32_t i2 = indices_[i + 2];
      add_edge(i0, i1);
      add_edge(i1, i2);
      add_edge(i2, i0);
    }

    ibh_lines_ = bgfx::createIndexBuffer(
        bgfx::makeRef(line_indices_.data(),
                      line_indices_.size() * sizeof(uint32_t)),
        BGFX_BUFFER_INDEX32);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    uint64_t state =
        (utils::kAlphaState & ~BGFX_STATE_PT_MASK) | BGFX_STATE_PT_LINES;
    bgfx::setState(state);
    ApplyColorUniforms();
    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, vbh_);
    bgfx::setIndexBuffer(ibh_lines_);
    bgfx::submit(0, program);
  }

 private:
  bgfx::VertexBufferHandle vbh_ = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh_ = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle ibh_lines_ = BGFX_INVALID_HANDLE;
  std::vector<glm::vec3> vertices_;
  std::vector<uint32_t> indices_;
  std::vector<uint32_t> line_indices_;
};
}  // namespace livision
