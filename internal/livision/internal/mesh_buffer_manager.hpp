#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "livision/MeshBuffer.hpp"
#include "livision/Vertex.hpp"

namespace livision::internal {

class MeshBufferManager {
 public:
  using MeshFactory = std::function<std::shared_ptr<MeshBuffer>()>;

  static std::shared_ptr<MeshBuffer> AcquireShared(const std::string& key,
                                                   const MeshFactory& factory);
  static std::shared_ptr<MeshBuffer> CreateTracked(std::vector<Vertex> vertices,
                                                   std::vector<uint32_t> indices,
                                                   bool has_uv = false);
  static void Register(const std::shared_ptr<MeshBuffer>& mesh);
  static void DestroyAllBuffers();
  static void SetBgfxAlive(bool alive);
  static bool IsBgfxAlive();
};

}  // namespace livision::internal
