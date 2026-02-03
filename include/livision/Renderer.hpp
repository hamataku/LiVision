#pragma once

#include <memory>

#include "MeshData.hpp"
#include "ObjectBase.hpp"

namespace livision {

class Renderer {
 public:
  Renderer();
  ~Renderer();
  void Init();
  void DeInit();

  void Submit(const MeshData& mesh_data, const Eigen::Affine3d& mtx,
              const Color& color);

 private:
  static void PrintBackend();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision