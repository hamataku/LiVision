#pragma once

#include "FastLS/LidarSim.hpp"
#include "FastLS/SceneBase.hpp"
#include "FastLS/object/PointCloud.hpp"
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

    plane_.SetSize(glm::vec2(40.0F, 40.0F)).SetColor(utils::white);
    AddObject(&plane_);

    mesh_.SetSize(glm::vec3(50.0F, 50.0F, 50.0F))
        .SetPos(glm::vec3(0.0F, 0.0F, -2.0F))
        .SetDegRotation(glm::vec3(90.0F, 0.0F, 0.0F))
        .SetColor(utils::light_gray);

    AddObject(&mesh_);

    AddObject(&point_cloud_);
  }

  void Update() override {
    // drone_.SetPos(glm::vec3(std::cos(container_theta_) * 6.0F,
    //                         std::sin(container_theta_) * 6.0F, 2.0F));
    drone_.SetPos(glm::vec3(6.0F, 0.0F, 2.0F))
        .SetRadRotation(glm::vec3(0.0F, 0.0F, container_theta_));
    container_theta_ += 0.005F;

    std::vector<glm::vec3> points;
    lidar_sim.GetPointCloud(points);
    point_cloud_.SetPoints(points);
  }

 private:
  fastls::Drone drone_;

  float container_theta_ = 0.0F;

  fastls::Plane plane_;
  fastls::PointCloud point_cloud_;
  fastls::Mesh mesh_{"data/bunny/bun_zipper_res3.stl"};
};
}  // namespace fastls