#pragma once

#include "livision/MeshData.hpp"
#include "livision/ObjectBase.hpp"
#include "livision/Renderer.hpp"

namespace livision {
class Plane : public ObjectBase {
 public:
  void InitImpl() final {
    static std::shared_ptr<MeshData> mesh = std::make_shared<MeshData>(
        std::vector<Eigen::Vector3f>{
            {-0.5F, 0.5F, 0.0F},   // top-left
            {0.5F, 0.5F, 0.0F},    // top-right
            {-0.5F, -0.5F, 0.0F},  // bottom-left
            {0.5F, -0.5F, 0.0F}    // bottom-right
        },
        std::vector<uint32_t>{0, 1, 2, 1, 3, 2, 0, 2, 1, 1, 2, 3});
    mesh_ = mesh;
  }

  void Draw(Renderer& renderer) final {
    renderer.Submit(*mesh_, global_mtx_, color_);
  }
};

}  // namespace livision
