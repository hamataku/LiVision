#include "FastLS/FastLS.hpp"
#include "FastLS/marker/PointCloud.hpp"
#include "FastLS/obstacle/DroneLidarUp.hpp"
#include "FastLS/obstacle/Mesh.hpp"
#include "FastLS/obstacle/Plane.hpp"
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

  fastls::PointCloud<> point_cloud;
  fastls::DroneLidarUp drone;
  drone.lidar_.AddObject(&point_cloud);
  fast_ls.AddObject(&drone);

  fastls::Plane plane;
  plane.SetSize(glm::vec2(40.0F, 40.0F)).SetColor(fastls::utils::white);
  fast_ls.AddObject(&plane);

  fastls::Mesh mesh("data/bunny/bun_zipper_res4.stl");
  mesh.SetSize(glm::dvec3(50.0, 50.0, 50.0))
      .SetPos(glm::dvec3(0.0, 0.0, -2.0))
      .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
      .SetColor(fastls::utils::light_gray);
  fast_ls.AddObject(&mesh);

  while (fast_ls.SpinOnce()) {
    drone
        .SetPos(glm::dvec3(std::cos(container_theta) * 6.0,
                           std::sin(container_theta) * 6.0, 2.0))
        .SetRadRotation(glm::dvec3(0.0, container_theta * 2, container_theta));

    container_theta += 0.01F;

    point_cloud.SetPoints(drone.lidar_.GetPointClouds());
  }

  return 0;
}
