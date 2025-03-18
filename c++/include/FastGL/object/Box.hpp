#pragma once

#include <array>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {

class Box : public ObjectBase {
 public:
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
    bgfx::setVertexBuffer(0, utils::cube_vbh);
    bgfx::setIndexBuffer(utils::cube_ibh);
    bgfx::submit(0, program);
  }

  void SetSize(const glm::vec3& size) { size_ = size; }
  void SetColor(std::array<float, 4>& color) {
    std::memcpy(color_, color.data(), sizeof(color_));
  }

 private:
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  float color_[4] = {1.0F, 0.0F, 0.0F, 1.0F};  // RGBA
};
}  // namespace fastgl
