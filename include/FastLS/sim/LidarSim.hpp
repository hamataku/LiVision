#pragma once

#include <bgfx/bgfx.h>

#include <random>

#include "FastLS/sensor/LidarSensor.hpp"

namespace fastls {

class LidarSim {
 public:
  LidarSim() : compute_program_(BGFX_INVALID_HANDLE) {}

  void Init();

  void Destroy();
  void RegisterLidar(LidarSensor* lidar_sensor);
  void AddMeshLists(const std::vector<glm::vec3>& vertex,
                    const std::vector<uint32_t>& index, const glm::mat4& mtx);

  void CalcPointCloud();

 private:
  static constexpr float kLidarStep = 3.0F;

  std::vector<glm::vec4> mesh_vertices_;

  bgfx::ProgramHandle compute_program_;
  bgfx::DynamicVertexBufferHandle mesh_buffer_;
  bgfx::DynamicVertexBufferHandle ray_dir_buffer_;
  bgfx::DynamicVertexBufferHandle position_buffer_;
  bgfx::DynamicVertexBufferHandle mtx_inv_buffer_;
  bgfx::DynamicVertexBufferHandle mtx_random_buffer_;
  bgfx::DynamicVertexBufferHandle lidar_range_buffer_;

  // static constexpr int kBufferCount = 2;
  // int frame_index_ = 0;  // バッファ切り替え用
  // bgfx::TextureHandle compute_texture_[kBufferCount];
  bgfx::TextureHandle compute_texture_;

  std::vector<glm::vec4> ray_dirs_;
  std::vector<glm::vec4> positions_;
  std::vector<glm::mat4> mtx_invs_;
  std::vector<glm::mat4> mtx_randoms_;
  std::vector<float> lidar_ranges_;

  int num_rays_;

  bgfx::UniformHandle u_params_;

  std::random_device rd_;
  std::mt19937 gen_{rd_()};
  std::uniform_real_distribution<double> random_hfov_{0, 360};
  std::uniform_real_distribution<double> random_vfov_{-1, 1};

  float* output_buffer_ = nullptr;

  std::vector<LidarSensor*> lidar_sensors_;
};

inline LidarSim lidar_sim;
}  // namespace fastls
