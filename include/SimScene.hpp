#pragma once

#include "FastLS/PointCloud.hpp"
#include "FastLS/SceneBase.hpp"
#include "FastLS/SimLidar.hpp"
#include "FastLS/utils.hpp"
// Object
#include "FastLS/object/Drone.hpp"
#include "FastLS/object/Mesh.hpp"
#include "FastLS/object/Plane.hpp"

namespace fastls {

class SimScene : public SceneBase {
 public:
  void Init() override {
    AddObject(&drone_);

    plane0_.SetSize(glm::vec2(40.0F, 40.0F));
    AddObject(&plane0_);

    mesh0_.SetSize(glm::vec3(50.0F, 50.0F, 50.0F))
        .SetPos(glm::vec3(0.0F, 0.0F, -2.0F))
        .SetDegRotation(glm::vec3(90.0F, 0.0F, 0.0F))
        .SetColor(utils::light_gray);
    // .SetVisible(false);
    AddObject(&mesh0_);

    AddObject(&point_cloud0_);
  }

  void Update() override {
    drone_.SetRadRotation(glm::vec3(container_theta_, container_theta_, 0.0F))
        .SetPos(glm::vec3(std::cos(container_theta_) * 6.0F,
                          std::sin(container_theta_) * 6.0F, 2.0F));
    container_theta_ += 0.02F;

    std::vector<glm::vec3> points;
    sim_lidar.GetPointCloud(points);
    point_cloud0_.SetPoints(points);
  }

 private:
  fastls::Drone drone_;

  float container_theta_ = 0.0F;

  fastls::Plane plane0_;
  fastls::PointCloud point_cloud0_;
  fastls::Mesh mesh0_{"data/bunny/bun_zipper_res3.stl"};
};
}  // namespace fastls