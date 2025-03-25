#pragma once

#include <bgfx/bgfx.h>

#include <glm/glm.hpp>
#include <iostream>
#include <random>
#include <vector>

#include "FastLS/object/ObjectBase.hpp"
#include "file_ops.hpp"
#include "utils.hpp"

namespace fastls {

class ObjectBase;

class SimLidar {
 public:
  SimLidar() : compute_program_(BGFX_INVALID_HANDLE) {}
  ~SimLidar() { Destroy(); }

  void Init() {
    std::cout << "SimLidar Init" << std::endl;

    for (float i = 0.0F; i < 360.0F; i += kLidarStep) {
      for (float j = 0.0F; j < 60.0F; j += 6.0F) {
        float rad_yaw = glm::radians(i);
        float rad_pitch = glm::radians(j);

        glm::vec4 dir = glm::vec4(std::cos(rad_yaw) * std::cos(rad_pitch),
                                  std::sin(rad_yaw) * std::cos(rad_pitch),
                                  std::sin(rad_pitch), 1.0F);
        ray_dirs_.push_back(dir);
      }
    }
    num_rays_ = ray_dirs_.size();

    const std::string shader_root = "shader/build/";

    std::string shader;
    if (!file_ops::ReadFile(shader_root + "compute_ray_cast.bin", shader)) {
      std::cout << "Could not find compute shader" << std::endl;
      return;
    }

    const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
    const bgfx::ShaderHandle handle = bgfx::createShader(mem);
    bgfx::setName(handle, "compute_ray_cast");

    compute_program_ =
        bgfx::createProgram(handle, true /* destroy shader on completion */);

    // GPUバッファの初期化（meshのサイズに合わせた固定バッファ）
    auto mesh_count = static_cast<uint32_t>(mesh_vertices_.size());

    std::cout << "mesh_count: " << mesh_count / 3 << std::endl;

    mesh_buffer_ = bgfx::createDynamicVertexBuffer(
        mesh_count, utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

    // レイ情報用のバッファ
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

    // ユニフォームの初期化
    u_params_ = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);
    u_mtx_ = bgfx::createUniform("u_mtx", bgfx::UniformType::Mat4);
    u_mtx_inv_ = bgfx::createUniform("u_mtx_inv", bgfx::UniformType::Mat4);
    u_mtx_lidar_ = bgfx::createUniform("u_mtx_lidar", bgfx::UniformType::Mat4);

    // メッシュデータのアップロード
    const bgfx::Memory* vertex_mem = bgfx::makeRef(
        mesh_vertices_.data(), mesh_vertices_.size() * sizeof(glm::vec4));
    bgfx::update(mesh_buffer_, 0, vertex_mem);

    // レイデータのアップロード
    const bgfx::Memory* ray_dir_mem =
        bgfx::makeRef(ray_dirs_.data(), ray_dirs_.size() * sizeof(glm::vec4));
    bgfx::update(ray_dir_buffer_, 0, ray_dir_mem);
  }

  void RegisterLidar(ObjectBase* lidar) { lidars_.push_back(lidar); }

  void AddMeshLists(const std::vector<glm::vec3>& vertex,
                    const std::vector<uint32_t>& index, const glm::mat4 mtx) {
    for (const auto& i : index) {
      glm::vec3 v = vertex[i];
      glm::vec4 result = mtx * glm::vec4(v, 1.0F);
      mesh_vertices_.push_back(result);
    }
  }

  void GetPointCloud(std::vector<glm::vec3>& points) {
    if (lidars_.empty()) return;
    glm::mat4 mtx = lidars_[0]->GetGlobalMatrix();
    glm::vec3 origin = glm::vec3(mtx[3][0], mtx[3][1], mtx[3][2]);
    glm::mat4 mtx_inv = glm::inverse(mtx);
    glm::mat4 mtx_lidar;

    float x_angle = random_vfov_(gen_);
    float y_angle = random_vfov_(gen_);
    float z_angle = random_hfov_(gen_);

    // lidarをランダムに回転させる変換行列
    mtx_lidar = glm::rotate(glm::mat4(1.0F), glm::radians(x_angle),
                            glm::vec3(1.0F, 0.0F, 0.0F));
    mtx_lidar = glm::rotate(mtx_lidar, glm::radians(y_angle),
                            glm::vec3(0.0F, 1.0F, 0.0F));
    mtx_lidar = glm::rotate(mtx_lidar, glm::radians(z_angle),
                            glm::vec3(0.0F, 0.0F, 1.0F));

    int prev_index = 1 - frame_index_;  // 前のフレームのインデックス

    // コンピュートシェーダーのセットアップと実行
    bgfx::setBuffer(0, mesh_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(1, ray_dir_buffer_, bgfx::Access::Read);
    bgfx::setImage(2, compute_texture_[frame_index_], 0, bgfx::Access::Write,
                   bgfx::TextureFormat::RGBA32F);

    float params[4] = {mesh_vertices_.size() / 3.0F, origin.x, origin.y,
                       origin.z};
    bgfx::setUniform(u_params_, params);
    bgfx::setUniform(u_mtx_, glm::value_ptr(mtx));
    bgfx::setUniform(u_mtx_inv_, glm::value_ptr(mtx_inv));
    bgfx::setUniform(u_mtx_lidar_, glm::value_ptr(mtx_lidar));

    constexpr uint32_t kThreadsX = 256;
    uint32_t num_groups_x = (num_rays_ + kThreadsX - 1) / kThreadsX;

    bgfx::dispatch(0, compute_program_, num_groups_x, 1, 1);

    // テクスチャからデータを読み出し
    bgfx::readTexture(compute_texture_[prev_index], output_buffer_);

    // 結果の処理
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
  void GetPointCloudCPU(std::vector<glm::vec3>& points) {
    points.clear();
    if (lidars_.empty()) return;
    glm::mat4 mtx = lidars_[0]->GetGlobalMatrix();
    glm::vec3 origin = glm::vec3(mtx[3][0], mtx[3][1], mtx[3][2]);
    glm::mat4 mtx_inv = glm::inverse(mtx);

    // For each ray direction
    for (size_t ray_idx = 0; ray_idx < num_rays_; ++ray_idx) {
      glm::vec3 ray_dir = ray_dirs_[ray_idx] * mtx_inv;

      float closest_t = std::numeric_limits<float>::max();
      bool hit = false;
      glm::vec3 intersection_point;

      // Loop through all triangles in the mesh
      for (size_t tri_idx = 0; tri_idx < mesh_vertices_.size() / 3; ++tri_idx) {
        glm::vec3 v0 = mesh_vertices_[tri_idx * 3];
        glm::vec3 v1 = mesh_vertices_[(tri_idx * 3) + 1];
        glm::vec3 v2 = mesh_vertices_[(tri_idx * 3) + 2];

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

 private:
  static constexpr int kBufferCount = 2;
  int frame_index_ = 0;  // バッファ切り替え用

  static constexpr float kLidarStep = 6.0F;

  std::vector<glm::vec4> mesh_vertices_;

  bgfx::ProgramHandle compute_program_;
  bgfx::DynamicVertexBufferHandle mesh_buffer_;
  bgfx::DynamicVertexBufferHandle ray_dir_buffer_;
  bgfx::TextureHandle compute_texture_[kBufferCount];

  std::vector<glm::vec4> ray_dirs_;
  int num_rays_;

  bgfx::UniformHandle u_params_;
  bgfx::UniformHandle u_mtx_;
  bgfx::UniformHandle u_mtx_inv_;
  bgfx::UniformHandle u_mtx_lidar_;

  std::random_device rd_;
  std::mt19937 gen_{rd_()};
  std::uniform_real_distribution<double> random_hfov_{0, 360};
  std::uniform_real_distribution<double> random_vfov_{-1, 1};

  float* output_buffer_ = nullptr;

  std::vector<ObjectBase*> lidars_;

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

  void Destroy() {
    if (bgfx::isValid(compute_program_)) {
      bgfx::destroy(compute_program_);
      compute_program_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(mesh_buffer_)) {
      bgfx::destroy(mesh_buffer_);
      mesh_buffer_ = BGFX_INVALID_HANDLE;
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

    if (bgfx::isValid(u_mtx_)) {
      bgfx::destroy(u_mtx_);
      u_mtx_ = BGFX_INVALID_HANDLE;
    }

    if (bgfx::isValid(u_mtx_inv_)) {
      bgfx::destroy(u_mtx_inv_);
      u_mtx_inv_ = BGFX_INVALID_HANDLE;
    }

    if (bgfx::isValid(u_mtx_lidar_)) {
      bgfx::destroy(u_mtx_lidar_);
      u_mtx_lidar_ = BGFX_INVALID_HANDLE;
    }
  }
};

inline SimLidar sim_lidar;
}  // namespace fastls
