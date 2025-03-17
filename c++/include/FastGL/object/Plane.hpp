#pragma once

#include <array>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {

class Plane : public ObjectBase {
 public:
  void Init() final {
    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .end();

    vbh_ = bgfx::createVertexBuffer(
        bgfx::makeRef(kPlaneVertices, sizeof(kPlaneVertices)),
        pos_col_vert_layout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(kPlaneTriList, sizeof(kPlaneTriList)));
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, color_);
    float mtx[16];
    bx::mtxIdentity(mtx);
    bx::mtxScale(mtx, size_.x, size_.y, 1.0F);
    bgfx::setTransform(mtx);
    bgfx::setVertexBuffer(0, vbh_);
    bgfx::setIndexBuffer(ibh_);
    bgfx::submit(0, program);
  }

  void SetSize(const glm::vec2& size) { size_ = size; }

  void SetColor(std::array<float, 4>& color) {
    std::memcpy(color_, color.data(), sizeof(color_));
  }

 private:
  glm::vec2 size_ = glm::vec2(1.0F, 1.0F);
  float color_[4] = {0.3F, 0.3F, 0.3F, 0.1F};  // RGBA

  static constexpr utils::PosVertex kPlaneVertices[4] = {
      {-0.5F, 0.5F, 0.0F},   // top-left
      {0.5F, 0.5F, 0.0F},    // top-right
      {-0.5F, -0.5F, 0.0F},  // bottom-left
      {0.5F, -0.5F, 0.0F}    // bottom-right
  };

  static constexpr uint16_t kPlaneTriList[12] = {0, 1, 2, 1, 3, 2,
                                                 0, 2, 1, 1, 2, 3};
};

}  // namespace fastgl
