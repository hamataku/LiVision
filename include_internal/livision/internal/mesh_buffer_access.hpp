#pragma once

#include <bgfx/bgfx.h>

#include "livision/MeshBuffer.hpp"

namespace livision::internal {

struct MeshBufferAccess {
  static bgfx::VertexBufferHandle VertexBuffer(MeshBuffer& mesh);
  static bgfx::IndexBufferHandle IndexBuffer(MeshBuffer& mesh);
  static bgfx::IndexBufferHandle WireIndexBuffer(MeshBuffer& mesh);
  static uint32_t GetIndexCount(MeshBuffer& mesh);
};

}  // namespace livision::internal
