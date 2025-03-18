#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/embedded_shader.h>

#include <glm/glm.hpp>
#include <iostream>
#include <vector>

#include "file_ops.hpp"
#include "object/Utils.hpp"

namespace fastgl {

class RayCast {
 public:
  RayCast() : compute_program_(BGFX_INVALID_HANDLE) {}
  ~RayCast() { Destroy(); }

  void Init() {
    std::cout << "RayCast Init" << std::endl;
    const std::string shader_root = "shader/build/";

    std::string shader;
    if (!fileops::ReadFile(shader_root + "compute_ray_cast.bin", shader)) {
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
        vertex_count, utils::pos_vert_layout, BGFX_BUFFER_COMPUTE_READ);
    index_buffer_ =
        bgfx::createDynamicIndexBuffer(index_count, BGFX_BUFFER_COMPUTE_READ);

    // レイ情報用のバッファ
    bgfx::VertexLayout layout;
    layout.begin()
        .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
        .end();

    // 十分な初期サイズでバッファを作成
    constexpr uint32_t kInitialSize = 4096;
    ray_origin_buffer_ = bgfx::createDynamicVertexBuffer(
        kInitialSize, layout, BGFX_BUFFER_COMPUTE_READ);
    ray_dir_buffer_ = bgfx::createDynamicVertexBuffer(kInitialSize, layout,
                                                      BGFX_BUFFER_COMPUTE_READ);

    // 結果バッファの初期化（テクスチャとして作成）
    result_texture_ = bgfx::createTexture2D(
        kInitialSize, 1, false, 1, bgfx::TextureFormat::RGBA32F,
        BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_READ_BACK);

    // リザルトバッファの格納用メモリを確保
    result_staging_ = static_cast<glm::vec4*>(
        std::aligned_alloc(16, kInitialSize * sizeof(glm::vec4)));
    result_staging_size_ = kInitialSize;

    u_params_ = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);
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
    if (bgfx::isValid(ray_origin_buffer_)) {
      bgfx::destroy(ray_origin_buffer_);
      ray_origin_buffer_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(ray_dir_buffer_)) {
      bgfx::destroy(ray_dir_buffer_);
      ray_dir_buffer_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(result_texture_)) {
      bgfx::destroy(result_texture_);
      result_texture_ = BGFX_INVALID_HANDLE;
    }
    if (result_staging_) {
      std::free(result_staging_);
      result_staging_ = nullptr;
    }

    if (bgfx::isValid(u_params_)) {
      bgfx::destroy(u_params_);
      u_params_ = BGFX_INVALID_HANDLE;
    }
  }

  static utils::PosVertex CalcMul(const utils::PosVertex* _vec,
                                  const float* _mat) {
    utils::PosVertex result;
    result.x =
        _vec->x * _mat[0] + _vec->y * _mat[4] + _vec->z * _mat[8] + _mat[12];
    result.y =
        _vec->x * _mat[1] + _vec->y * _mat[5] + _vec->z * _mat[9] + _mat[13];
    result.z =
        _vec->x * _mat[2] + _vec->y * _mat[6] + _vec->z * _mat[10] + _mat[14];
    return result;
  }

  void AddMeshLists(const utils::PosVertex* vertices, float* mtx,
                    const uint16_t* indices, uint32_t num_vertices,
                    uint32_t num_indices) {
    for (uint32_t i = 0; i < num_vertices; ++i) {
      utils::PosVertex result = CalcMul(&vertices[i], mtx);
      mesh_vertices_.push_back(result);
    }
    for (uint32_t i = 0; i < num_indices; ++i) {
      mesh_indices_.push_back(mesh_index_ + indices[i]);
    }
    mesh_index_ += num_vertices;
  }

  void GetPointCloud(std::vector<glm::vec3>& points, const glm::vec3& origin,
                     const std::vector<glm::vec3>& direction) {
    auto num_rays = static_cast<uint32_t>(direction.size());

    // バッファのリサイズが必要かチェック
    if (num_rays > result_staging_size_) {
      std::free(result_staging_);
      result_staging_size_ = num_rays;
      result_staging_ = static_cast<glm::vec4*>(
          std::aligned_alloc(16, result_staging_size_ * sizeof(glm::vec4)));

      // バッファとテクスチャの再作成
      if (bgfx::isValid(ray_origin_buffer_)) {
        bgfx::destroy(ray_origin_buffer_);
      }
      if (bgfx::isValid(ray_dir_buffer_)) {
        bgfx::destroy(ray_dir_buffer_);
      }
      if (bgfx::isValid(result_texture_)) {
        bgfx::destroy(result_texture_);
      }

      bgfx::VertexLayout layout;
      layout.begin()
          .add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
          .end();

      ray_origin_buffer_ = bgfx::createDynamicVertexBuffer(
          num_rays, layout, BGFX_BUFFER_COMPUTE_READ);
      ray_dir_buffer_ = bgfx::createDynamicVertexBuffer(
          num_rays, layout, BGFX_BUFFER_COMPUTE_READ);
      result_texture_ = bgfx::createTexture2D(
          num_rays, 1, false, 1, bgfx::TextureFormat::RGBA32F,
          BGFX_TEXTURE_COMPUTE_WRITE | BGFX_TEXTURE_READ_BACK);
    }

    // メッシュデータのアップロード
    const bgfx::Memory* vertex_mem =
        bgfx::makeRef(mesh_vertices_.data(),
                      mesh_vertices_.size() * sizeof(utils::PosVertex));
    bgfx::update(vertex_buffer_, 0, vertex_mem);

    const bgfx::Memory* index_mem = bgfx::makeRef(
        mesh_indices_.data(), mesh_indices_.size() * sizeof(uint16_t));
    bgfx::update(index_buffer_, 0, index_mem);

    // レイ情報の準備
    std::vector<glm::vec4> ray_origins(num_rays, glm::vec4(origin, 0.0F));
    std::vector<glm::vec4> ray_dirs;
    ray_dirs.reserve(num_rays);
    for (const auto& dir : direction) {
      ray_dirs.emplace_back(glm::normalize(dir), 0.0F);
    }

    // レイデータのアップロード
    const bgfx::Memory* ray_origin_mem = bgfx::makeRef(
        ray_origins.data(), ray_origins.size() * sizeof(glm::vec4));
    const bgfx::Memory* ray_dir_mem =
        bgfx::makeRef(ray_dirs.data(), ray_dirs.size() * sizeof(glm::vec4));

    bgfx::update(ray_origin_buffer_, 0, ray_origin_mem);
    bgfx::update(ray_dir_buffer_, 0, ray_dir_mem);

    // コンピュートシェーダーのセットアップと実行
    bgfx::setBuffer(0, vertex_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(1, index_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(2, ray_origin_buffer_, bgfx::Access::Read);
    bgfx::setBuffer(3, ray_dir_buffer_, bgfx::Access::Read);
    bgfx::setImage(4, result_texture_, 0, bgfx::Access::Write,
                   bgfx::TextureFormat::RGBA32F);

    float params[4] = {static_cast<float>(mesh_indices_.size()), 0.0F, 0.0F,
                       0.0F};
    bgfx::setUniform(u_params_, params);

    uint32_t num_threads_x = (num_rays + 255) & ~255;
    bgfx::dispatch(0, compute_program_, num_threads_x / 256, 1, 1);

    // 結果の読み取り
    bgfx::frame();  // フレームを進めて計算完了を待つ

    // テクスチャからデータを読み出し
    bgfx::readTexture(result_texture_, result_staging_);
    bgfx::frame();  // データ転送完了を待つ

    // 結果の処理
    points.clear();
    for (size_t i = 0; i < num_rays; ++i) {
      if (result_staging_[i].w > 0.0F) {  // 交差があった場合
        points.emplace_back(result_staging_[i].x, result_staging_[i].y,
                            result_staging_[i].z);
      }
    }
  }

 private:
  std::vector<utils::PosVertex> mesh_vertices_;
  std::vector<uint16_t> mesh_indices_;
  uint16_t mesh_index_ = 0;

  bgfx::ProgramHandle compute_program_;
  bgfx::DynamicVertexBufferHandle vertex_buffer_;
  bgfx::DynamicIndexBufferHandle index_buffer_;
  bgfx::DynamicVertexBufferHandle ray_origin_buffer_;
  bgfx::DynamicVertexBufferHandle ray_dir_buffer_;
  bgfx::TextureHandle result_texture_;

  bgfx::UniformHandle u_params_;

  glm::vec4* result_staging_ = nullptr;
  size_t result_staging_size_ = 0;
};

inline RayCast ray_cast;
}  // namespace fastgl
