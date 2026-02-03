#pragma once

#include <Eigen/Geometry>
#include <memory>

#include "Color.hpp"
#include "MeshBuffer.hpp"

namespace livision {

class Renderer {
 public:
  Renderer();
  ~Renderer();
  void Init();
  void DeInit();

  void Submit(const MeshBuffer& mesh_data, const Eigen::Affine3d& mtx,
              const Color& color);

 private:
  static void PrintBackend();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision