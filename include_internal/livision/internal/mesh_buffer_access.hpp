#pragma once

#include <bgfx/bgfx.h>

#include "livision/MeshBuffer.hpp"

namespace livision::internal {

struct MeshBufferAccess {
  static bgfx::VertexBufferHandle VertexBuffer(const MeshBuffer& mesh);
  static bgfx::IndexBufferHandle IndexBuffer(const MeshBuffer& mesh);
  static uint32_t IndexCount(const MeshBuffer& mesh);
};

}  // namespace livision::internal
