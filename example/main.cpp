#include "livision/Viewer.hpp"
// #include "livision/marker/PointCloud.hpp"
// #include "livision/obstacle/DroneLidarUp.hpp"
// #include "livision/obstacle/Mesh.hpp"
#include "livision/obstacle/Plane.hpp"
// #include "livision/obstacle/WireFrame.hpp"
// #include "livision/utils.hpp"

#include <iostream>

int main() {
  livision::Viewer fast_ls{{
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
      .fps = true,        // Target frames per second
  }};

  // constexpr int kNumDrones = 5;

  // std::array<livision::PointCloud<>, kNumDrones> point_clouds;
  // std::array<livision::DroneLidarUp, kNumDrones> drones;
  // std::array<double, kNumDrones> container_thetas;

  // for (int i = 0; i < kNumDrones; ++i) {
  //   container_thetas[i] = (2.0 * M_PI) / static_cast<double>(kNumDrones) * i;
  //   drones[i]
  //       .SetPos(glm::dvec3(std::cos(container_thetas[i]) * 6.0,
  //                          std::sin(container_thetas[i]) * 6.0, 2.0))
  //       .SetRadRotation(
  //           glm::dvec3(0.0, container_thetas[i] * 2, container_thetas[i]));
  //   point_clouds[i].SetPointSize(0.5F).SetColorSpec(
  //       livision::utils::color_palette[i % 10]);
  //   drones[i].lidar_.AddObject(&point_clouds[i]);
  //   fast_ls.AddObject(&drones[i]);
  // }

  livision::Plane plane;
  plane.SetSize(Eigen::Vector3d(40.0F, 40.0F, 0.0F))
      .SetColor(livision::color::white);
  fast_ls.AddObject(&plane);

  // livision::Mesh mesh("data/bunny/bun_zipper_res3.stl");
  // mesh.SetSize(glm::dvec3(50.0, 50.0, 50.0))
  //     .SetPos(glm::dvec3(0.0, 0.0, -2.0))
  //     .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
  //     .SetColorSpec(livision::utils::rainbow_z);
  // fast_ls.AddObject(&mesh);

  // livision::WireFrame wireframe("data/bunny/bun_zipper_res3.stl");
  // wireframe.SetSize(glm::dvec3(50.0, 50.0, 50.0))
  //     .SetPos(glm::dvec3(0.0, 0.0, -2.0))
  //     .SetDegRotation(glm::dvec3(90.0, 0.0, 0.0))
  //     .SetColorSpec(livision::utils::black);
  // fast_ls.AddObject(&wireframe);

  while (fast_ls.SpinOnce()) {
    // for (int i = 0; i < kNumDrones; ++i) {
    //   drones[i]
    //       .SetPos(glm::dvec3(std::cos(container_thetas[i]) * 6.0,
    //                          std::sin(container_thetas[i]) * 6.0, 2.0))
    //       .SetRadRotation(
    //           glm::dvec3(0.0, container_thetas[i] * 2, container_thetas[i]));
    //   container_thetas[i] += 0.01F;
    //   point_clouds[i].SetPoints(drones[i].lidar_.GetPointClouds());
    // }
  }

  return 0;
}
