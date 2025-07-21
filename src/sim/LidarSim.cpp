#include "FastLS/sim/LidarSim.hpp"

#include "FastLS/file_ops.hpp"

namespace fastls {
void LidarSim::Init() {
  for (float i = 0.0F; i < 360.0F; i += kLidarStep) {
    for (float j = 0.0F; j < 60.0F; j += 2.0F) {
      float rad_yaw = glm::radians(i);
      float rad_pitch = glm::radians(j);

      glm::vec4 dir = glm::vec4(std::cos(rad_yaw) * std::cos(rad_pitch),
                                std::sin(rad_yaw) * std::cos(rad_pitch),
                                std::sin(rad_pitch), 1.0F);
      ray_dirs_.push_back(dir);
    }
  }
  num_rays_ = ray_dirs_.size();

  positions_.resize(lidar_sensors_.size());
  mtx_invs_.resize(lidar_sensors_.size());
  mtx_randoms_.resize(lidar_sensors_.size());
  lidar_ranges_.resize(lidar_sensors_.size());

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

  mesh_buffer_ = bgfx::createDynamicVertexBuffer(
      mesh_count, utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

  // レイ情報用のバッファ
  ray_dir_buffer_ = bgfx::createDynamicVertexBuffer(
      num_rays_, utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

  // lidar位置用のバッファ
  position_buffer_ = bgfx::createDynamicVertexBuffer(
      lidar_sensors_.size(), utils::vec4_vlayout, BGFX_BUFFER_COMPUTE_READ);

  // lidarの逆行列用のバッファ
  mtx_inv_buffer_ = bgfx::createDynamicVertexBuffer(
      lidar_sensors_.size(), utils::mat4_vlayout, BGFX_BUFFER_COMPUTE_READ);

  // ランダムな回転行列用のバッファ
  mtx_random_buffer_ = bgfx::createDynamicVertexBuffer(
      lidar_sensors_.size(), utils::mat4_vlayout, BGFX_BUFFER_COMPUTE_READ);

  // lidarの最大距離用のバッファ
  lidar_range_buffer_ = bgfx::createDynamicVertexBuffer(
      lidar_sensors_.size(), utils::float_vlayout, BGFX_BUFFER_COMPUTE_READ);

  // 結果バッファの初期化
  // for (auto& i : compute_texture_) {
  //   i = bgfx::createTexture2D(num_rays_, lidar_sensors_.size(), false, 1,
  //                             bgfx::TextureFormat::RGBA32F,
  //                             BGFX_TEXTURE_COMPUTE_WRITE);
  // }
  compute_texture_ = bgfx::createTexture2D(
      num_rays_, static_cast<uint32_t>(lidar_sensors_.size()), false, 1,
      bgfx::TextureFormat::RGBA32F, BGFX_TEXTURE_COMPUTE_WRITE);

  // 結果用メモリを確保
  output_buffer_ = static_cast<float*>(
      std::aligned_alloc(16, static_cast<size_t>(num_rays_) * sizeof(float) *
                                 4 * lidar_sensors_.size()));

  // ユニフォームの初期化
  u_params_ = bgfx::createUniform("u_params", bgfx::UniformType::Vec4);

  // メッシュデータのアップロード
  const bgfx::Memory* vertex_mem = bgfx::makeRef(
      mesh_vertices_.data(), mesh_vertices_.size() * sizeof(glm::vec4));
  bgfx::update(mesh_buffer_, 0, vertex_mem);

  // レイデータのアップロード
  const bgfx::Memory* ray_dir_mem =
      bgfx::makeRef(ray_dirs_.data(), ray_dirs_.size() * sizeof(glm::vec4));
  bgfx::update(ray_dir_buffer_, 0, ray_dir_mem);
}

void LidarSim::Destroy() {
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
  if (bgfx::isValid(position_buffer_)) {
    bgfx::destroy(position_buffer_);
    position_buffer_ = BGFX_INVALID_HANDLE;
  }
  if (bgfx::isValid(mtx_inv_buffer_)) {
    bgfx::destroy(mtx_inv_buffer_);
    mtx_inv_buffer_ = BGFX_INVALID_HANDLE;
  }
  if (bgfx::isValid(mtx_random_buffer_)) {
    bgfx::destroy(mtx_random_buffer_);
    mtx_random_buffer_ = BGFX_INVALID_HANDLE;
  }
  if (bgfx::isValid(lidar_range_buffer_)) {
    bgfx::destroy(lidar_range_buffer_);
    lidar_range_buffer_ = BGFX_INVALID_HANDLE;
  }

  // for (auto& i : compute_texture_) {
  //   if (bgfx::isValid(i)) {
  //     bgfx::destroy(i);
  //     i = BGFX_INVALID_HANDLE;
  //   }
  // }
  if (bgfx::isValid(compute_texture_)) {
    bgfx::destroy(compute_texture_);
    compute_texture_ = BGFX_INVALID_HANDLE;
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

void LidarSim::RegisterLidar(LidarSensor* lidar_sensor) {
  lidar_sensors_.push_back(lidar_sensor);
}

void LidarSim::AddMeshLists(const std::vector<glm::vec3>& vertex,
                            const std::vector<uint32_t>& index,
                            const glm::mat4& mtx) {
  for (const auto& i : index) {
    glm::vec3 v = vertex[i];
    glm::vec4 result = mtx * glm::vec4(v, 1.0F);
    mesh_vertices_.push_back(result);
  }
}

void LidarSim::CalcPointCloud() {
  if (lidar_sensors_.empty()) {
    bgfx::frame();
    return;
  }

  // LiDARデータのパッキング
  for (size_t i = 0; i < lidar_sensors_.size(); ++i) {
    glm::mat4 mtx = lidar_sensors_[i]->GetGlobalMatrix();
    positions_[i] = glm::vec4(mtx[3][0], mtx[3][1], mtx[3][2], 0.0F);

    mtx_invs_[i] = glm::inverse(mtx);

    float x_angle = random_vfov_(gen_);
    float y_angle = random_vfov_(gen_);
    float z_angle = random_hfov_(gen_);

    // lidarをランダムに回転させる変換行列
    mtx_randoms_[i] = glm::rotate(glm::mat4(1.0F), glm::radians(x_angle),
                                  glm::vec3(1.0F, 0.0F, 0.0F));
    mtx_randoms_[i] = glm::rotate(mtx_randoms_[i], glm::radians(y_angle),
                                  glm::vec3(0.0F, 1.0F, 0.0F));
    mtx_randoms_[i] = glm::rotate(mtx_randoms_[i], glm::radians(z_angle),
                                  glm::vec3(0.0F, 0.0F, 1.0F));

    lidar_ranges_[i] = lidar_sensors_[i]->GetLidarRange();
  }

  // バッファの更新
  const bgfx::Memory* position_mem =
      bgfx::makeRef(positions_.data(), positions_.size() * sizeof(glm::vec4));
  bgfx::update(position_buffer_, 0, position_mem);
  const bgfx::Memory* mtx_inv_mem =
      bgfx::makeRef(mtx_invs_.data(), mtx_invs_.size() * sizeof(glm::mat4));
  bgfx::update(mtx_inv_buffer_, 0, mtx_inv_mem);
  const bgfx::Memory* mtx_random_mem = bgfx::makeRef(
      mtx_randoms_.data(), mtx_randoms_.size() * sizeof(glm::mat4));
  bgfx::update(mtx_random_buffer_, 0, mtx_random_mem);
  const bgfx::Memory* lidar_range_mem =
      bgfx::makeRef(lidar_ranges_.data(), lidar_ranges_.size() * sizeof(float));
  bgfx::update(lidar_range_buffer_, 0, lidar_range_mem);

  // コンピュートシェーダーのセットアップと実行
  bgfx::setBuffer(0, mesh_buffer_, bgfx::Access::Read);
  bgfx::setBuffer(1, ray_dir_buffer_, bgfx::Access::Read);
  bgfx::setBuffer(2, position_buffer_, bgfx::Access::Read);
  bgfx::setBuffer(3, mtx_inv_buffer_, bgfx::Access::Read);
  bgfx::setBuffer(4, mtx_random_buffer_, bgfx::Access::Read);
  bgfx::setBuffer(5, lidar_range_buffer_, bgfx::Access::Read);
  bgfx::setImage(6, compute_texture_, 0, bgfx::Access::Write,
                 bgfx::TextureFormat::RGBA32F);

  float params[4] = {mesh_vertices_.size() / 3.0F,
                     static_cast<float>(num_rays_),
                     static_cast<float>(lidar_sensors_.size()), 0.0F};
  bgfx::setUniform(u_params_, params);

  constexpr uint32_t kThreadsX = 64;
  constexpr uint32_t kThreadsY = 16;
  uint32_t num_groups_x = (num_rays_ + kThreadsX - 1) / kThreadsX;
  uint32_t num_groups_y = (lidar_sensors_.size() + kThreadsY - 1) / kThreadsY;

  // 計算要求
  bgfx::dispatch(0, compute_program_, num_groups_x, num_groups_y, 1);

  // frame_index_ = 1 - frame_index_;  // バッファを切り替える

  // テクスチャからデータを読み出し要求
  bgfx::readTexture(compute_texture_, output_buffer_);
  bgfx::frame();

  // 結果の処理
  for (size_t i = 0; i < lidar_sensors_.size(); ++i) {
    size_t start_index = i * num_rays_;
    lidar_sensors_[i]->GetPointClouds().clear();
    for (size_t j = 0; j < num_rays_; ++j) {
      size_t index = (start_index + j) * 4;
      if (output_buffer_[index + 3] > 0.0F) {  // 交差があった場合
        glm::vec3 point(output_buffer_[index], output_buffer_[index + 1],
                        output_buffer_[index + 2]);
        lidar_sensors_[i]->GetPointClouds().emplace_back(point);
      }
    }
  }
}

}  // namespace fastls
