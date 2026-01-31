#pragma once

#include "FastLS/SceneBase.hpp"
// Object
#include "FastLS/object/marker/PointCloud.hpp"
#include "FastLS/object/obstacle/DroneLidarUp.hpp"
#include "FastLS/object/obstacle/Mesh.hpp"
#include "FastLS/object/obstacle/Plane.hpp"

namespace fastls {

class SimScene : public SceneBase {
 public:
  void Init() override {
    drone_.lidar_.AddObject(&point_cloud_);
    AddObject(&drone_);

    plane_.SetSize(glm::vec2(40.0F, 40.0F)).SetColor(utils::white);
    AddObject(&plane_);

    mesh_.SetSize(glm::dvec3(50.0, 50.0, 50.0))
        .SetPos(glm::dvec3(0.0, 0.0, -2.0))
        .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
        .SetColor(utils::light_gray);

    AddObject(&mesh_);
  }

  void Update() override {
    drone_
        .SetPos(glm::dvec3(std::cos(container_theta_) * 6.0,
                           std::sin(container_theta_) * 6.0, 2.0))
        .SetRadRotation(
            glm::dvec3(0.0, container_theta_ * 2, container_theta_));
    container_theta_ += 0.01F;

    point_cloud_.SetPoints(drone_.lidar_.GetPointClouds());
  }

 private:
  fastls::DroneLidarUp drone_;

  double container_theta_ = 0.0;

  fastls::Plane plane_;
  fastls::PointCloud<> point_cloud_;
  fastls::Mesh mesh_{"data/bunny/bun_zipper_res3.stl"};
};
}  // namespace fastls