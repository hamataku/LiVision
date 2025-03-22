#pragma once

#include "FastLS/PointCloud.hpp"
#include "FastLS/SceneBase.hpp"
#include "FastLS/SimLidar.hpp"
#include "FastLS/object/Box.hpp"
#include "FastLS/object/Mesh.hpp"
#include "FastLS/object/Plane.hpp"

namespace fastls {

class SimScene : public SceneBase {
 public:
  void Init() override {
    utils::Color yellow{0.8F, 0.7F, 0.3F, 0.1F};

    box0_.SetSize(glm::vec3(0.1F, 10.0F, 5.0F))
        .SetPos(glm::vec3(5.0F, 0.0F, 2.5F))
        .SetColor(yellow);
    AddObject(&box0_);

    box1_.SetSize(glm::vec3(0.1F, 10.0F, 5.0F))
        .SetPos(glm::vec3(-5.0F, 0.0F, 2.5F))
        .SetColor(yellow);
    AddObject(&box1_);

    box2_.SetSize(glm::vec3(10.0F, 0.1F, 5.0F))
        .SetPos(glm::vec3(0.0F, 5.0F, 2.5F))
        .SetColor(yellow);
    AddObject(&box2_);

    box3_.SetSize(glm::vec3(10.0F, 0.1F, 5.0F))
        .SetPos(glm::vec3(0.0F, -5.0F, 2.5F))
        .SetColor(yellow);
    AddObject(&box3_);

    // for (auto& box : boxs_) {
    //   box.SetSize(glm::vec3(0.1F, 0.1F, 0.1F));
    //   box.SetPos(glm::vec3(0.0F, 0.0F, 0.0F));
    //   box.SetColor(color);
    //   AddObject(&box);
    // }

    plane0_.SetSize(glm::vec2(10.0F, 10.0F));
    AddObject(&plane0_);

    mesh0_.SetPos(glm::vec3(0.0F, 0.0F, 1.0F)).SetColor(yellow);
    AddObject(&mesh0_);

    AddObject(&point_cloud0_);
  }
  void Update() override {
    std::vector<glm::vec3> points;
    const glm::vec3 origin{0.0F, 4.0F, 1.0F};
    sim_lidar.GetPointCloud(points, origin);
    point_cloud0_.SetPoints(points);
  }

 private:
  fastls::Box box0_;
  fastls::Box box1_;
  fastls::Box box2_;
  fastls::Box box3_;
  // std::array<fastls::Box, 200> boxs_;
  fastls::Plane plane0_;
  fastls::PointCloud point_cloud0_;
  fastls::Mesh mesh0_{"data/box.stl"};
};
}  // namespace fastls