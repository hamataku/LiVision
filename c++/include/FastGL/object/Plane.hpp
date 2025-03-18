#pragma once

#include <array>
#include <cstring>
#include <glm/gtc/type_ptr.hpp>

#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {

class Plane : public ObjectBase {
 public:
  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, color_);
    float mtx[16];
    bx::mtxIdentity(mtx);
    bx::mtxScale(mtx, size_.x, size_.y, 1.0F);
    bgfx::setTransform(mtx);
    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }

  void SetSize(const glm::vec2& size) { size_ = size; }

  void SetColor(std::array<float, 4>& color) {
    std::memcpy(color_, color.data(), sizeof(color_));
  }

 private:
  glm::vec2 size_ = glm::vec2(1.0F, 1.0F);
  float color_[4] = {0.3F, 0.3F, 0.3F, 0.1F};  // RGBA
};

}  // namespace fastgl
