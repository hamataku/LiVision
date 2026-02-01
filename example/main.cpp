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

  constexpr int kNumDrones = 5;

  std::array<fastls::PointCloud<>, kNumDrones> point_clouds;
  std::array<fastls::DroneLidarUp, kNumDrones> drones;
  std::array<double, kNumDrones> container_thetas;

  for (int i = 0; i < kNumDrones; ++i) {
    container_thetas[i] = (2.0 * M_PI) / static_cast<double>(kNumDrones) * i;
    drones[i]
        .SetPos(glm::dvec3(std::cos(container_thetas[i]) * 6.0,
                           std::sin(container_thetas[i]) * 6.0, 2.0))
        .SetRadRotation(
            glm::dvec3(0.0, container_thetas[i] * 2, container_thetas[i]));
    point_clouds[i].SetPointSize(0.5F).SetColorSpec(
        fastls::utils::color_palette[i % 10]);
    drones[i].lidar_.AddObject(&point_clouds[i]);
    fast_ls.AddObject(&drones[i]);
  }

  fastls::Plane plane;
  plane.SetSize(glm::vec2(40.0F, 40.0F)).SetColorSpec(fastls::utils::white);
  fast_ls.AddObject(&plane);

  fastls::Mesh mesh("data/bunny/bun_zipper_res3.stl");
  mesh.SetSize(glm::dvec3(50.0, 50.0, 50.0))
      .SetPos(glm::dvec3(0.0, 0.0, -2.0))
      .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
      .SetColorSpec(fastls::utils::rainbow_z);
  fast_ls.AddObject(&mesh);

  fastls::WireFrame wireframe("data/bunny/bun_zipper_res3.stl");
  wireframe.SetSize(glm::dvec3(50.0, 50.0, 50.0))
      .SetPos(glm::dvec3(0.0, 0.0, -2.0))
      .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
      .SetColorSpec(fastls::utils::black);
  fast_ls.AddObject(&wireframe);

  while (fast_ls.SpinOnce()) {
    for (int i = 0; i < kNumDrones; ++i) {
      drones[i]
          .SetPos(glm::dvec3(std::cos(container_thetas[i]) * 6.0,
                             std::sin(container_thetas[i]) * 6.0, 2.0))
          .SetRadRotation(
              glm::dvec3(0.0, container_thetas[i] * 2, container_thetas[i]));
      container_thetas[i] += 0.01F;
      point_clouds[i].SetPoints(drones[i].lidar_.GetPointClouds());
    }
  }

  return 0;
}
