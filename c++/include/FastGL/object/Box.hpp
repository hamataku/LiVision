#pragma once

#include <array>
#include <glm/gtc/type_ptr.hpp>

#include "../RayCast.hpp"
#include "ObjectBase.hpp"
#include "Utils.hpp"

namespace fastgl {

class Box : public ObjectBase {
 public:
  void CalcMtx(float* mtx) {
    bx::mtxScale(mtx, size_.x, size_.y, size_.z);
    mtx[12] = pos_.x;
    mtx[13] = pos_.y;
    mtx[14] = pos_.z;
  }

  void AddMeshList() override {
    float mtx[16];
    CalcMtx(mtx);
    ray_cast.AddMeshLists(utils::kCubeVertices, mtx, utils::kCubeTriList, 8,
                          36);
  }

  void Draw(bgfx::ProgramHandle& program) final {
    bgfx::setUniform(utils::u_color, color_.data());
    // Create transformation matrix
    float mtx[16];
    CalcMtx(mtx);

    bgfx::setTransform(mtx);
    bgfx::setVertexBuffer(0, utils::cube_vbh);
    bgfx::setIndexBuffer(utils::cube_ibh);
    bgfx::submit(0, program);
  }

  void SetSize(const glm::vec3& size) { size_ = size; }
  void SetColor(std::array<float, 4>& color) { color_ = color; }

 private:
  glm::vec3 size_ = glm::vec3(1.0F, 1.0F, 1.0F);
  std::array<float, 4> color_{1.0F, 0.0F, 0.0F, 1.0F};  // RGBA
};
}  // namespace fastgl
