#pragma once

#include "FastLS/PointCloud.hpp"
#include "FastLS/SceneBase.hpp"
#include "FastLS/SimLidar.hpp"
#include "FastLS/object/Box.hpp"
#include "FastLS/object/Mesh.hpp"
#include "FastLS/object/Plane.hpp"
#include "FastLS/utils.hpp"

namespace fastls {

class SimScene : public SceneBase {
 public:
  void Init() override {
    plane0_.SetSize(glm::vec2(10.0F, 10.0F));
    AddObject(&plane0_);

    mesh0_.SetSize(glm::vec3(50.0F, 50.0F, 50.0F))
        .SetPos(glm::vec3(0.0F, 4.0F, -2.0F))
        .SetDegRotation(glm::vec3(90.0F, 0.0F, 0.0F))
        .SetColor(utils::light_gray);
    AddObject(&mesh0_);

    AddObject(&point_cloud0_);
  }
  void Update() override {
    std::vector<glm::vec3> points;
    const glm::vec3 origin{0.0F, -4.0F, 1.0F};
    sim_lidar.GetPointCloud(points, origin);
    point_cloud0_.SetPoints(points);
  }

 private:
  fastls::Box box0_;
  fastls::Box box1_;
  fastls::Box box2_;
  fastls::Box box3_;
  fastls::Plane plane0_;
  fastls::PointCloud point_cloud0_;
  fastls::Mesh mesh0_{"data/bunny/bun_zipper_res2.stl"};
};
}  // namespace fastls