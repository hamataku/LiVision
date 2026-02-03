#pragma once

#include <bgfx/bgfx.h>

#include "livision/MeshData.hpp"

namespace livision::internal {

struct MeshDataAccess {
  static bgfx::VertexBufferHandle VertexBuffer(const MeshData& mesh);
  static bgfx::IndexBufferHandle IndexBuffer(const MeshData& mesh);
  static uint32_t IndexCount(const MeshData& mesh);
};

}  // namespace livision::internal
