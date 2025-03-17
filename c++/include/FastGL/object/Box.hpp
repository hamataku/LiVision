#pragma once

#include <array>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {
struct PosVertex {
  float x;
  float y;
  float z;
};

class Box : public ObjectBase {
 public:
  void Init() final {
    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .end();

    vbh_ = bgfx::createVertexBuffer(
        bgfx::makeRef(kCubeVertices, sizeof(kCubeVertices)),
        pos_col_vert_layout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(kCubeTriList, sizeof(kCubeTriList)));
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, color_);
    // Create transformation matrix
    float mtx[16];
    bx::mtxIdentity(mtx);
    bx::mtxScale(mtx, size_.x, size_.y, size_.z);
    // const float* quat = glm::value_ptr(rotation_);
    // bx::mtxRotateX(mtx, quat[0]);
    // bx::mtxRotateY(mtx, quat[1]);
    // bx::mtxRotateZ(mtx, quat[2]);
    // bx::mtxTranslate(mtx, pos_.x, pos_.y, pos_.z);

    bgfx::setTransform(mtx);
    bgfx::setVertexBuffer(0, vbh_);
    bgfx::setIndexBuffer(ibh_);
    bgfx::submit(0, program);
  }

  void SetSize(const glm::vec3& size) { size_ = size; }
  void SetColor(std::array<float, 4>& color) {
    std::memcpy(color_, color.data(), sizeof(color_));
  }

 private:
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  float color_[4] = {1.0F, 0.0F, 0.0F, 1.0F};  // RGBA

  static constexpr PosVertex kCubeVertices[8] = {
      {-1.0F, 1.0F, 1.0F},   {1.0F, 1.0F, 1.0F},   {-1.0F, -1.0F, 1.0F},
      {1.0F, -1.0F, 1.0F},   {-1.0F, 1.0F, -1.0F}, {1.0F, 1.0F, -1.0F},
      {-1.0F, -1.0F, -1.0F}, {1.0F, -1.0F, -1.0F},
  };

  static constexpr uint16_t kCubeTriList[36] = {
      0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
      1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
  };
};
}  // namespace fastgl
