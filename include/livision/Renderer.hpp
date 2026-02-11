#pragma once

#include <Eigen/Geometry>
#include <memory>
#include <string>
#include <vector>

#include "livision/Color.hpp"
#include "livision/MeshBuffer.hpp"

namespace livision {

/**
 * @brief Low-level rendering backend wrapper.
 */
class Renderer {
 public:
  /**
   * @brief Construct renderer instance (not initialized).
   */
  Renderer();
  /**
   * @brief Destroy renderer and release resources.
   */
  ~Renderer();
  /**
   * @brief Initialize graphics backend.
   */
  void Init();
  /**
   * @brief Deinitialize graphics backend.
   */
  void DeInit();
  /**
   * @brief Set directories used to search for shaders.
   */
  void SetShaderSearchPaths(std::vector<std::string> paths);

  /**
   * @brief Submit a mesh with transform and colors.
   */
  void Submit(MeshBuffer& mesh_buffer, const Eigen::Affine3d& mtx,
              const Color& color, const std::string& texture,
              const Color& wire_color);
  /**
   * @brief Submit instanced draws with per-instance positions.
   */
  void SubmitInstanced(MeshBuffer& mesh_buffer,
                       const std::vector<Eigen::Vector4d>& points,
                       const Eigen::Affine3d& mtx, const Color& color);

 private:
  static void PrintBackend();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision
