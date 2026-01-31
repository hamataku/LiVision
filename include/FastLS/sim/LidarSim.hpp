#pragma once

#include <bgfx/bgfx.h>

#include <cstdint>
#include <random>

#include "FastLS/ObjectBase.hpp"
#include "FastLS/sensor/LidarSensor.hpp"

namespace fastls {

class LidarSim {
 public:
  LidarSim() : compute_program_(BGFX_INVALID_HANDLE) {}

  void Init();

  void Destroy();
  void RegisterLidar(LidarSensor* lidar_sensor);
  void InitMeshList(ObjectBase* object);
  void UpdateDynamicMeshList(ObjectBase* object);

  void CalcPointCloud();

 private:
  struct BvhNode {
    glm::vec3 bmin;
    glm::vec3 bmax;
    uint32_t left = 0;
    uint32_t right = 0;
    uint32_t first = 0;
    uint32_t count = 0;
    bool leaf = false;
  };

  void BuildBvh();

  static constexpr float kLidarStepH = 1.5F;
  static constexpr float kLidarStepV = 1.5F;
  static constexpr uint32_t kBvhLeafSize = 8;

  std::vector<glm::vec4> mesh_static_vertices_;
  std::vector<glm::vec4> mesh_dynamic_vertices_;
  size_t total_vertices_size_ = 0;

  bgfx::ProgramHandle compute_program_{};
  bgfx::DynamicVertexBufferHandle mesh_buffer_;
  bgfx::DynamicVertexBufferHandle ray_dir_buffer_;
  bgfx::DynamicVertexBufferHandle position_buffer_;
  bgfx::DynamicVertexBufferHandle mtx_inv_buffer_;
  bgfx::DynamicVertexBufferHandle mtx_random_buffer_;
  bgfx::DynamicVertexBufferHandle lidar_range_buffer_;
  bgfx::DynamicVertexBufferHandle bvh_node_buffer_;
  bgfx::DynamicVertexBufferHandle bvh_tri_index_buffer_;

  // static constexpr int kBufferCount = 2;
  // int frame_index_ = 0;  // バッファ切り替え用
  // bgfx::TextureHandle compute_texture_[kBufferCount];
  bgfx::TextureHandle compute_texture_;

  std::vector<glm::vec4> ray_dirs_;
  std::vector<glm::vec4> positions_;
  std::vector<glm::mat4> mtx_invs_;
  std::vector<glm::mat4> mtx_randoms_;
  std::vector<float> lidar_ranges_;

  size_t num_rays_;

  bgfx::UniformHandle u_params_;
  bgfx::UniformHandle u_bvh_params_;

  std::random_device rd_;
  std::mt19937 gen_{rd_()};
  std::uniform_real_distribution<double> random_hfov_{0, kLidarStepH};
  std::uniform_real_distribution<double> random_vfov_{-kLidarStepV,
                                                      kLidarStepV};

  float* output_buffer_ = nullptr;

  std::vector<BvhNode> bvh_nodes_;
  std::vector<uint32_t> bvh_tri_indices_;
  std::vector<glm::vec4> bvh_node_data_;
  std::vector<float> bvh_tri_index_data_;
  size_t bvh_node_capacity_ = 0;
  size_t bvh_tri_capacity_ = 0;
  bool bvh_initialized_ = false;

  std::vector<LidarSensor*> lidar_sensors_;
};

inline LidarSim lidar_sim;
}  // namespace fastls
