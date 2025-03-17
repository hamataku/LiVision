#pragma once

#include <cstring>
#include <glm/gtc/type_ptr.hpp>
#include "ObjectBase.hpp"

namespace fastgl {
struct PosColorVertex {
  float x;
  float y;
  float z;
  uint32_t abgr;
};

class Box : public ObjectBase {
 public:
  void Init() final {
    memcpy(vertices_, cube_vertices_, sizeof(cube_vertices_));
    for (auto& vertex : vertices_) {
      vertex.abgr = color_;
      vertex.x *= size_.x;
      vertex.y *= size_.y;
      vertex.z *= size_.z;
    }

    bgfx::VertexLayout pos_col_vert_layout;
    pos_col_vert_layout.begin()
        .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
        .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
        .end();

    vbh_ = bgfx::createVertexBuffer(bgfx::makeRef(vertices_, sizeof(vertices_)),
                                    pos_col_vert_layout);
    ibh_ = bgfx::createIndexBuffer(
        bgfx::makeRef(cube_tri_list_, sizeof(cube_tri_list_)));
  }

  void Draw(bgfx::ProgramHandle& program) final {
    // Create transformation matrix
    float mtx[16];
    bx::mtxIdentity(mtx);
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
  void SetColor(uint32_t color) { color_ = color; }

 private:
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  uint32_t color_ = 0xff0000ff;  // abgr

  const PosColorVertex cube_vertices_[8] = {
      {-1.0F, 1.0F, 1.0F, 0},   {1.0F, 1.0F, 1.0F, 0},
      {-1.0F, -1.0F, 1.0F, 0},  {1.0F, -1.0F, 1.0F, 0},
      {-1.0F, 1.0F, -1.0F, 0},  {1.0F, 1.0F, -1.0F, 0},
      {-1.0F, -1.0F, -1.0F, 0}, {1.0F, -1.0F, -1.0F, 0},
  };

  PosColorVertex vertices_[8];

  const uint16_t cube_tri_list_[36] = {
      0, 1, 2, 1, 3, 2, 4, 6, 5, 5, 6, 7, 0, 2, 4, 4, 2, 6,
      1, 5, 3, 5, 7, 3, 0, 4, 1, 4, 5, 1, 2, 3, 6, 6, 3, 7,
  };
};
}  // namespace fastgl
