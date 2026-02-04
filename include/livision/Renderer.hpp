#pragma once

#include <Eigen/Geometry>
#include <memory>
#include <string>
#include <vector>

#include "Color.hpp"
#include "MeshBuffer.hpp"

namespace livision {

class Renderer {
 public:
  Renderer();
  ~Renderer();
  void Init();
  void DeInit();
  void SetShaderSearchPaths(std::vector<std::string> paths);

  void Submit(MeshBuffer& mesh_buffer, const Eigen::Affine3d& mtx,
              const Color& color, const Color& wire_color);

 private:
  static void PrintBackend();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision