#include "FastLS/FastLS.hpp"
#include "FastLS/marker/PointCloud.hpp"
#include "FastLS/obstacle/DroneLidarUp.hpp"
#include "FastLS/obstacle/Mesh.hpp"
#include "FastLS/obstacle/Plane.hpp"
#include "FastLS/obstacle/WireFrame.hpp"
#include "FastLS/utils.hpp"

int main() {
  fastls::FastLS fast_ls{{
      .headless = false,  // Set to true for headless mode
      .vsync = false,     // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
      .fps = true,        // Target frames per second
  }};

  double container_theta = 0.0;

  fastls::PointCloud<fastls::Plane> point_cloud;
  fastls::DroneLidarUp drone;
  point_cloud.SetColorSpec(fastls::utils::blue);
  drone.lidar_.AddObject(&point_cloud);
  fast_ls.AddObject(&drone);

  fastls::Plane plane;
  plane.SetSize(glm::vec2(40.0F, 40.0F)).SetColorSpec(fastls::utils::white);
  fast_ls.AddObject(&plane);

  fastls::Mesh mesh("data/bunny/bun_zipper_res4.stl");
  mesh.SetSize(glm::dvec3(50.0, 50.0, 50.0))
      .SetPos(glm::dvec3(0.0, 0.0, -2.0))
      .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
      .SetColorSpec(fastls::utils::rainbow_z);
  fast_ls.AddObject(&mesh);

  fastls::WireFrame wireframe("data/bunny/bun_zipper_res4.stl");
  wireframe.SetSize(glm::dvec3(50.0, 50.0, 50.0))
      .SetPos(glm::dvec3(0.0, 0.0, -2.0))
      .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
      .SetColorSpec(fastls::utils::black);
  fast_ls.AddObject(&wireframe);

  while (fast_ls.SpinOnce()) {
    drone
        .SetPos(glm::dvec3(std::cos(container_theta) * 6.0,
                           std::sin(container_theta) * 6.0, 2.0))
        .SetRadRotation(glm::dvec3(0.0, container_theta * 2, container_theta));

    container_theta += 0.01F;

    auto& v = drone.lidar_.GetPointClouds();
    std::cout << "Point cloud size: " << v.size() << std::endl;
    if (v.size() < 2000) {
      point_cloud.SetPoints(v);
    } else {
      std::vector<glm::vec3> sub(v.begin() + 1, v.begin() + 1500);
      point_cloud.SetPoints(sub);
    }
  }

  return 0;
}
