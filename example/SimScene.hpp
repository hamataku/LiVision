#pragma once

#include "FastLS/SceneBase.hpp"
// Object
#include "FastLS/object/Drone.hpp"
#include "FastLS/object/Mesh.hpp"
#include "FastLS/object/Plane.hpp"
#include "FastLS/object/PointCloud.hpp"

namespace fastls {

class SimScene : public SceneBase {
 public:
  void Init() override {
    AddObject(&drone_);

    plane_.SetSize(glm::vec2(40.0F, 40.0F))
        .SetColor(utils::white)
        .SetForceVisible(true);
    AddObject(&plane_);

    mesh_.SetSize(glm::dvec3(50.0, 50.0, 50.0))
        .SetPos(glm::dvec3(0.0, 0.0, -2.0))
        .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
        .SetColor(utils::light_gray);

    AddObject(&mesh_);

    AddObject(&point_cloud_);
  }

  void Update() override {
    drone_
        .SetPos(glm::dvec3(std::cos(container_theta_) * 6.0,
                           std::sin(container_theta_) * 6.0, 2.0))
        .SetRadRotation(
            glm::dvec3(0.0, container_theta_ * 2, container_theta_));
    container_theta_ += 0.01F;

    point_cloud_.SetPoints(drone_.lidar_.GetPointClouds(),
                           drone_.lidar_.GetGlobalMatrix());
  }

 private:
  fastls::Drone drone_;

  double container_theta_ = 0.0;

  fastls::Plane plane_;
  fastls::PointCloud<> point_cloud_;
  fastls::Mesh mesh_{"data/bunny/bun_zipper_res4.stl"};
};
}  // namespace fastls