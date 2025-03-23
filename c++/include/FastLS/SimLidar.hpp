#pragma once

#include <bgfx/bgfx.h>

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "file_ops.hpp"
#include "utils.hpp"

namespace fastls {

class SimLidar {
 private:
  struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    void Expand(const glm::vec3& point) {
      min = glm::min(min, point);
      max = glm::max(max, point);
    }

    void Expand(const AABB& other) {
      min = glm::min(min, other.min);
      max = glm::max(max, other.max);
    }

    AABB()
        : min(std::numeric_limits<float>::max()),
          max(-std::numeric_limits<float>::max()) {}
  };

  struct BVHNode {
    AABB bounds;
    uint32_t left_child;   // 左子ノードのインデックス
    uint32_t right_child;  // 右子ノードのインデックス
    uint32_t first_tri;    // 最初の三角形インデックス
    uint32_t tri_count;    // 三角形の数
  };

  std::vector<BVHNode> bvh_nodes_;
  bgfx::DynamicVertexBufferHandle bvh_buffer_ = BGFX_INVALID_HANDLE;

  void BuildBVH();
  uint32_t CreateNode();
  void SplitNode(uint32_t node_idx, uint32_t start, uint32_t count,
                 const AABB& bounds, int depth);
  AABB ComputeSceneBounds() const;
  void CreateBVHBuffer();

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
        ray_dirs_.emplace_back(dir);
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
    auto mesh_count = static_cast<uint32_t>(mesh_vertices_.size());

    std::cout << "mesh_count: " << mesh_count / 3 << std::endl;

    mesh_buffer_ = bgfx::createDynamicVertexBuffer(
        mesh_count, utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

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
    // メッシュデータのアップロード後にBVHを構築
    const bgfx::Memory* vertex_mem = bgfx::makeRef(
        mesh_vertices_.data(), mesh_vertices_.size() * sizeof(glm::vec4));
    bgfx::update(mesh_buffer_, 0, vertex_mem);

    // BVHの構築とGPUバッファの作成
    BuildBVH();
    CreateBVHBuffer();

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
    if (bgfx::isValid(mesh_buffer_)) {
      bgfx::destroy(mesh_buffer_);
      mesh_buffer_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(ray_dir_buffer_)) {
      bgfx::destroy(ray_dir_buffer_);
      ray_dir_buffer_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(bvh_buffer_)) {
      bgfx::destroy(bvh_buffer_);
      bvh_buffer_ = BGFX_INVALID_HANDLE;
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
    for (const auto& i : index) {
      glm::vec3 v = vertex[i];
      glm::vec4 result = mtx * glm::vec4(v, 1.0F);
      mesh_vertices_.emplace_back(result);
    }
  }

  void GetPointCloud(std::vector<glm::vec3>& points, const glm::vec3& origin) {
    int prev_index = 1 - frame_index_;  // 前のフレームのインデックス

    // コンピュートシェーダーのセットアップと実行
    bgfx::setBuffer(0, mesh_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(1, ray_dir_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(2, bvh_buffer_, bgfx::Access::Read);
    bgfx::setImage(3, compute_texture_[frame_index_], 0, bgfx::Access::Write,
                   bgfx::TextureFormat::RGBA32F);

    float params[4] = {mesh_vertices_.size() / 3.0F, origin.x, origin.y,
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

    // デバッグカウンター
    size_t total_aabb_tests = 0;
    size_t total_tri_tests = 0;
    size_t total_hits = 0;

    if (bvh_nodes_.empty()) {
      std::cout << "Warning: BVH is empty, no points will be generated"
                << std::endl;
      return;
    }

    // レイ方向ごとのループ
    for (size_t ray_idx = 0; ray_idx < num_rays_; ++ray_idx) {
      glm::vec3 ray_dir(ray_dirs_[ray_idx].x, ray_dirs_[ray_idx].y,
                        ray_dirs_[ray_idx].z);

      float closest_t = std::numeric_limits<float>::max();
      bool hit = false;
      glm::vec3 intersection_point;

      // BVHトラバーサル用のスタック
      std::vector<uint32_t> stack;
      stack.reserve(64);   // スタックの初期サイズを確保
      stack.push_back(0);  // ルートノードから開始

      while (!stack.empty()) {
        uint32_t node_idx = stack.back();
        stack.pop_back();

        const BVHNode& node = bvh_nodes_[node_idx];

        total_aabb_tests++;
        // AABBとの交差判定
        bool hit_box =
            IntersectAABB(origin, ray_dir, node.bounds.min, node.bounds.max);
        if (!hit_box) {
          continue;
        }

        if (node.tri_count > 0) {
          // 葉ノード: 三角形との交差判定
          for (uint32_t i = 0; i < node.tri_count; ++i) {
            total_tri_tests++;
            uint32_t tri_idx = node.first_tri + i;
            glm::vec3 v0 = mesh_vertices_[tri_idx * 3];
            glm::vec3 v1 = mesh_vertices_[tri_idx * 3 + 1];
            glm::vec3 v2 = mesh_vertices_[tri_idx * 3 + 2];

            float t;
            if (RayTriangleIntersection(origin, ray_dir, v0, v1, v2, t)) {
              if (t < closest_t) {
                closest_t = t;
                hit = true;
                intersection_point = origin + ray_dir * t;
              }
            }
          }
        } else {
          // 内部ノード: 子ノードをスタックに追加
          stack.push_back(node.right_child);
          stack.push_back(node.left_child);
        }
      }

      // 交差点が見つかった場合、追加
      if (hit) {
        points.emplace_back(intersection_point);
        total_hits++;
      }
    }
  }

  // Ray-AABB intersection test
  static bool IntersectAABB(const glm::vec3& origin, const glm::vec3& dir,
                            const glm::vec3& box_min,
                            const glm::vec3& box_max) {
    glm::vec3 inv_dir = 1.0F / dir;
    glm::vec3 t0 = (box_min - origin) * inv_dir;
    glm::vec3 t1 = (box_max - origin) * inv_dir;
    glm::vec3 tmin = glm::min(t0, t1);
    glm::vec3 tmax = glm::max(t0, t1);
    float tenter = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    float texit = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
    return tenter <= texit && texit > 0.0F;
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

  bgfx::ProgramHandle compute_program_;
  bgfx::DynamicVertexBufferHandle mesh_buffer_;
  bgfx::DynamicVertexBufferHandle ray_dir_buffer_;
  bgfx::TextureHandle compute_texture_[kBufferCount];

  std::vector<glm::vec4> gpu_nodes_;

  std::vector<glm::vec4> ray_dirs_;
  int num_rays_;

  bgfx::UniformHandle u_params_;

  float* output_buffer_ = nullptr;
};

inline SimLidar sim_lidar;
}  // namespace fastls
