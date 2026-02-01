#pragma once

#include <bgfx/bgfx.h>
#include <bgfx/defines.h>

#include <cmath>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "FastLS/ObjectBase.hpp"
#include "FastLS/obstacle/Box.hpp"

namespace fastls {

template <class T = Box>
class PointCloud : public ObjectBase {
 public:
  ~PointCloud() override {
    if (bgfx::isValid(mesh_vbh_)) {
      bgfx::destroy(mesh_vbh_);
      mesh_vbh_ = BGFX_INVALID_HANDLE;
    }
    if (bgfx::isValid(mesh_ibh_)) {
      bgfx::destroy(mesh_ibh_);
      mesh_ibh_ = BGFX_INVALID_HANDLE;
    }
  }

  void Draw(bgfx::ProgramHandle& program) final { DrawPointSprites(program); }

  PointCloud& SetVoxelSize(float size) {
    voxel_size_ = size;
    return *this;
  }

  PointCloud& SetPointSize(float size) {
    point_size_ = size;
    return *this;
  }

  PointCloud& SetPoints(const std::vector<glm::vec3>& points) {
    points_.clear();
    for (const auto& p : points) {
      points_.emplace_back(p.x, p.y, p.z, voxel_size_);
    }
    return *this;
  }

  PointCloud& SetPoints(const std::vector<glm::vec4>& points) {
    points_ = points;
    return *this;
  }

  const std::vector<glm::vec4>& GetPoints() { return points_; }

 private:
  void DrawPointSprites(bgfx::ProgramHandle& program) {
    if (points_.empty()) return;

    if (!bgfx::isValid(mesh_vbh_) || !bgfx::isValid(mesh_ibh_)) {
      const utils::MeshView view = obj_.GetMeshView();
      mesh_vbh_ = bgfx::createVertexBuffer(
          bgfx::makeRef(view.vertices.data(),
                        view.vertices.size() * sizeof(glm::vec3)),
          utils::vec3_vlayout);
      mesh_ibh_ = bgfx::createIndexBuffer(
          bgfx::makeRef(view.indices.data(),
                        view.indices.size() * sizeof(uint32_t)),
          BGFX_BUFFER_INDEX32);
      mesh_index_count_ = static_cast<uint32_t>(view.indices.size());
    }

    bgfx::InstanceDataBuffer idb;
    const uint32_t instance_count = static_cast<uint32_t>(points_.size());
    const uint16_t instance_stride = sizeof(float) * 4;
    if (bgfx::getAvailInstanceDataBuffer(instance_count, instance_stride) <
        instance_count) {
      return;
    }
    bgfx::allocInstanceDataBuffer(&idb, instance_count, instance_stride);

    float* data = reinterpret_cast<float*>(idb.data);
    for (const auto& p : points_) {
      data[0] = p.x;
      data[1] = p.y;
      data[2] = p.z;
      data[3] = static_cast<float>(p.w * point_size_);
      data += 4;
    }

    bgfx::setState(utils::kAlphaState);
    ApplyColorUniforms();

    auto mtx = glm::mat4(global_mtx_);
    bgfx::setTransform(glm::value_ptr(mtx));

    bgfx::setVertexBuffer(0, mesh_vbh_);
    if (mesh_index_count_ > 0) {
      bgfx::setIndexBuffer(mesh_ibh_, 0, mesh_index_count_);
    }
    bgfx::setInstanceDataBuffer(&idb);

    if (bgfx::isValid(utils::point_program)) {
      bgfx::submit(0, utils::point_program);
    } else {
      bgfx::submit(0, program);
    }
  }

  std::vector<glm::vec4> points_;
  double voxel_size_ = 0.12;
  float point_size_ = 1.0F;
  bgfx::VertexBufferHandle mesh_vbh_ = BGFX_INVALID_HANDLE;
  bgfx::IndexBufferHandle mesh_ibh_ = BGFX_INVALID_HANDLE;
  uint32_t mesh_index_count_ = 0;
  T obj_;
};

}  // namespace fastls
