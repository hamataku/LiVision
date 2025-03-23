#pragma once

#include <bgfx/bgfx.h>

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "file_ops.hpp"
#include "utils.hpp"

namespace fastls {

class SimLidar {
 public:
  SimLidar() : compute_program_(BGFX_INVALID_HANDLE) {}
  ~SimLidar() { Destroy(); }

  void Init() {
    std::cout << "SimLidar Init" << std::endl;

    for (float i = 0.0F; i < 360.0F; i += 1.0F) {
      for (float j = 0.0F; j < 60.0F; j += 1.5F) {
        float rad_yaw = glm::radians(static_cast<float>(i));
        float rad_pitch = glm::radians(static_cast<float>(j));

        glm::vec3 dir_normalized = glm::normalize(glm::vec3(
            std::cos(rad_yaw), std::sin(rad_yaw), std::sin(rad_pitch)));
        glm::vec4 dir = glm::vec4(dir_normalized, 0.0F);
        ray_dirs_.push_back(dir);
      }
    }
    num_rays_ = ray_dirs_.size();

    const std::string shader_root = "shader/build/";

    std::string shader;
    if (!file_ops::ReadFile(shader_root + "compute_ray_cast.bin", shader)) {
      printf("Could not find compute shader");
      return;
    }

    const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, "compute_ray_cast");

    compute_program_ =
        bgfx::createProgram(handle, true /* destroy shader on completion */);

    // GPUバッファの初期化（meshのサイズに合わせた固定バッファ）
    auto vertex_count = static_cast<uint32_t>(mesh_vertices_.size());
    auto index_count = static_cast<uint32_t>(mesh_indices_.size());

    std::cout << "vertex_count: " << vertex_count << std::endl;
    std::cout << "index_count: " << index_count << std::endl;

    vertex_buffer_ = bgfx::createDynamicVertexBuffer(
        vertex_count, utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);
    index_buffer_ = bgfx::createDynamicIndexBuffer(
        index_count, BGFX_BUFFER_COMPUTE_READ | BGFX_BUFFER_INDEX32);

    // レイ情報用のバッファ
    // 十分な初期サイズでバッファを作成
    ray_dir_buffer_ = bgfx::createDynamicVertexBuffer(
        num_rays_, utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

    // 結果バッファの初期化
    for (auto& i : compute_texture_) {
      i = bgfx::createTexture2D(num_rays_, 1, false, 1,
                                bgfx::TextureFormat::RGBA32F,
                                BGFX_TEXTURE_COMPUTE_WRITE);
    }

    // 結果用メモリを確保
    output_buffer_ = static_cast<float*>(
        std::aligned_alloc(16, num_rays_ * sizeof(float) * 4));

    u_params_ = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);

    // メッシュデータのアップロード
    const bgfx::Memory* vertex_mem = bgfx::makeRef(
        mesh_vertices_.data(), mesh_vertices_.size() * sizeof(glm::vec4));
    bgfx::update(vertex_buffer_, 0, vertex_mem);

    const bgfx::Memory* index_mem = bgfx::makeRef(
        mesh_indices_.data(), mesh_indices_.size() * sizeof(uint32_t));
    bgfx::update(index_buffer_, 0, index_mem);

    // レイデータのアップロード
    const bgfx::Memory* ray_dir_mem =
        bgfx::makeRef(ray_dirs_.data(), ray_dirs_.size() * sizeof(glm::vec4));
    bgfx::update(ray_dir_buffer_, 0, ray_dir_mem);
  }

  void Destroy() {
    if (bgfx::isValid(compute_program_)) {
      bgfx::destroy(compute_program_);
      compute_program_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(vertex_buffer_)) {
      bgfx::destroy(vertex_buffer_);
      vertex_buffer_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(index_buffer_)) {
      bgfx::destroy(index_buffer_);
      index_buffer_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(ray_dir_buffer_)) {
      bgfx::destroy(ray_dir_buffer_);
      ray_dir_buffer_ = BGFX_INVALID_HANDLE;
    }
    for (auto& i : compute_texture_) {
      if (bgfx::isValid(i)) {
        bgfx::destroy(i);
        i = BGFX_INVALID_HANDLE;
      }
    }
    if (output_buffer_) {
      std::free(output_buffer_);
      output_buffer_ = nullptr;
    }

    if (bgfx::isValid(u_params_)) {
      bgfx::destroy(u_params_);
      u_params_ = BGFX_INVALID_HANDLE;
    }
  }

  void AddMeshLists(const std::vector<glm::vec3>& vertex,
                    const std::vector<uint32_t>& index, const glm::mat4 mtx) {
    for (const auto& v : vertex) {
      glm::vec4 result = mtx * glm::vec4(v, 1.0F);
      mesh_vertices_.push_back(result);
    }
    for (const auto& i : index) {
      mesh_indices_.push_back(mesh_index_ + i);
    }
    mesh_index_ += static_cast<uint32_t>(vertex.size());
  }

  void GetPointCloud(std::vector<glm::vec3>& points, const glm::vec3& origin) {
    int prev_index = 1 - frame_index_;  // 前のフレームのインデックス

    // コンピュートシェーダーのセットアップと実行
    bgfx::setBuffer(0, vertex_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(1, index_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(2, ray_dir_buffer_, bgfx::Access::Read);
    bgfx::setImage(3, compute_texture_[frame_index_], 0, bgfx::Access::Write,
                   bgfx::TextureFormat::RGBA32F);

    float params[4] = {mesh_indices_.size() / 3.0F, origin.x, origin.y,
                       origin.z};
    bgfx::setUniform(u_params_, params);

    constexpr uint32_t kThreadsX = 256;
    uint32_t num_groups_x = (num_rays_ + kThreadsX - 1) / kThreadsX;

    bgfx::dispatch(0, compute_program_, num_groups_x, 1, 1);

    // テクスチャからデータを読み出し
    bgfx::readTexture(compute_texture_[prev_index], output_buffer_);

    // 結果の処理
    points.clear();
    for (size_t i = 0; i < num_rays_; ++i) {
      if (output_buffer_[(i * 4) + 3] > 0.0F) {  // 交差があった場合
        points.emplace_back(output_buffer_[i * 4], output_buffer_[(i * 4) + 1],
                            output_buffer_[(i * 4) + 2]);
      }
    }

    // バッファインデックスを切り替える
    frame_index_ = 1 - frame_index_;
  }

  // CPU-based ray casting implementation with the same interface
  void GetPointCloudCPU(std::vector<glm::vec3>& points,
                        const glm::vec3& origin) {
    points.clear();

    // For each ray direction
    for (size_t ray_idx = 0; ray_idx < num_rays_; ++ray_idx) {
      glm::vec3 ray_dir(ray_dirs_[ray_idx].x, ray_dirs_[ray_idx].y,
                        ray_dirs_[ray_idx].z);

      float closest_t = std::numeric_limits<float>::max();
      bool hit = false;
      glm::vec3 intersection_point;

      // Loop through all triangles in the mesh
      for (size_t tri_idx = 0; tri_idx < mesh_indices_.size() / 3; ++tri_idx) {
        uint32_t idx0 = mesh_indices_[tri_idx * 3];
        uint32_t idx1 = mesh_indices_[(tri_idx * 3) + 1];
        uint32_t idx2 = mesh_indices_[(tri_idx * 3) + 2];

        glm::vec3 v0(mesh_vertices_[idx0].x, mesh_vertices_[idx0].y,
                     mesh_vertices_[idx0].z);
        glm::vec3 v1(mesh_vertices_[idx1].x, mesh_vertices_[idx1].y,
                     mesh_vertices_[idx1].z);
        glm::vec3 v2(mesh_vertices_[idx2].x, mesh_vertices_[idx2].y,
                     mesh_vertices_[idx2].z);

        float t;
        if (RayTriangleIntersection(origin, ray_dir, v0, v1, v2, t)) {
          // Find closest intersection
          if (t < closest_t) {
            closest_t = t;
            hit = true;
            intersection_point = origin + ray_dir * t;
          }
        }
      }

      // Add intersection point if hit
      if (hit) {
        points.push_back(intersection_point);
      }
    }
  }

  // Ray-triangle intersection algorithm (Möller–Trumbore algorithm)
  bool RayTriangleIntersection(const glm::vec3& ray_origin,  // NOLINT
                               const glm::vec3& ray_dir, const glm::vec3& v0,
                               const glm::vec3& v1, const glm::vec3& v2,
                               float& t) const {
    constexpr float kEpsilon = 0.0000001F;

    glm::vec3 edge1 = v1 - v0;
    glm::vec3 edge2 = v2 - v0;
    glm::vec3 h = glm::cross(ray_dir, edge2);
    float a = glm::dot(edge1, h);

    // Ray parallel to triangle
    if (a > -kEpsilon && a < kEpsilon) return false;

    float f = 1.0F / a;
    glm::vec3 s = ray_origin - v0;
    float u = f * glm::dot(s, h);

    // Intersection outside triangle
    if (u < 0.0F || u > 1.0F) return false;

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray_dir, q);

    // Intersection outside triangle
    if (v < 0.0F || u + v > 1.0F) return false;

    // Calculate distance along ray
    t = f * glm::dot(edge2, q);

    // Only accept intersections in front of the ray
    return t > kEpsilon;
  }

 private:
  static constexpr int kBufferCount = 2;
  int frame_index_ = 0;  // バッファ切り替え用

  std::vector<glm::vec4> mesh_vertices_;
  std::vector<uint32_t> mesh_indices_;
  uint32_t mesh_index_ = 0;

  bgfx::ProgramHandle compute_program_;
  bgfx::DynamicVertexBufferHandle vertex_buffer_;
  bgfx::DynamicIndexBufferHandle index_buffer_;
  bgfx::DynamicVertexBufferHandle ray_dir_buffer_;
  bgfx::TextureHandle compute_texture_[kBufferCount];

  std::vector<glm::vec4> ray_dirs_;
  int num_rays_;

  bgfx::UniformHandle u_params_;

  float* output_buffer_ = nullptr;
};

inline SimLidar sim_lidar;
}  // namespace fastls
