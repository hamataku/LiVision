#pragma once

#include <array>
#include <glm/gtc/type_ptr.hpp>

#include "../RayCast.hpp"
#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {

class Plane : public ObjectBase {
 public:
  void CalcMtx(float* mtx) {
    bx::mtxScale(mtx, size_.x, size_.y, 1.0F);
    mtx[12] = pos_.x;
    mtx[13] = pos_.y;
    mtx[14] = pos_.z;
  }

  void AddMeshList() override {
    float mtx[16];
    CalcMtx(mtx);
    ray_cast.AddMeshLists(utils::kPlaneVertices, mtx, utils::kPlaneTriList, 4,
                          12);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, color_.data());
    float mtx[16];
    CalcMtx(mtx);

    bgfx::setTransform(mtx);

    bgfx::setVertexBuffer(0, utils::plane_vbh);
    bgfx::setIndexBuffer(utils::plane_ibh);
    bgfx::submit(0, program);
  }

  void SetSize(const glm::vec2& size) { size_ = size; }

  void SetColor(std::array<float, 4>& color) { color_ = color; }

 private:
  glm::vec2 size_ = glm::vec2(1.0F, 1.0F);
  std::array<float, 4> color_{0.3F, 0.3F, 0.3F, 0.1F};  // RGBA
};

}  // namespace fastgl
