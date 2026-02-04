#include <cmath>

#include "livision/Viewer.hpp"
// objects
#include "livision/marker/Arrow.hpp"
#include "livision/marker/DegeneracyIndicator.hpp"
#include "livision/marker/Grid.hpp"
#include "livision/marker/Odometry.hpp"
#include "livision/marker/Path.hpp"
#include "livision/marker/PointCloud.hpp"
#include "livision/object/Drone.hpp"
#include "livision/object/Mesh.hpp"
#include "livision/object/primitives.hpp"

int main() {
  namespace lv = livision;
  lv::Viewer viewer{{
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
      .fps = true,        // Target frames per second
  }};

  // UI callback
  viewer.RegisterUICallback([&viewer]() {
    if (ImGui::Button("Close")) {
      viewer.Close();
    }
  });

  // Plane
  lv::Plane plane({.scale = {40.0, 40.0, 0.0}, .color = lv::color::light_gray});
  viewer.AddObject(&plane);

  // Grid
  lv::Grid grid;
  grid.SetResolution(1.0).SetScale(Eigen::Vector3d(40.0, 40.0, 0.0));
  viewer.AddObject(&grid);

  // Mesh bunny
  lv::Mesh bunny("data/bunny/bun_zipper_res2.stl",
                 {.pos = {0.0, 0.0, -2.0},
                  .scale = {50.0, 50.0, 50.0},
                  .color = lv::color::rainbow_z,
                  .wire_color = lv::color::black});
  viewer.AddObject(&bunny.SetDegRotation({90.0, 0.0, 0.0}));

  // Box
  lv::Box box({.scale = {2.0, 2.0, 2.0}, .color = lv::color::rainbow_z});
  viewer.AddObject(&box);

  // Cone
  lv::Cone cone({.scale = {1.5, 1.5, 3.0},
                 .color = lv::color::green,
                 .wire_color = lv::color::black});
  viewer.AddObject(&cone);

  // Cylinder
  lv::Cylinder cylinder({.scale = {1.5, 1.5, 3.0},
                         .color = lv::color::orange,
                         .wire_color = lv::color::black});
  viewer.AddObject(&cylinder);

  // Sphere
  lv::Sphere sphere({.scale = {2.0, 2.0, 2.0},
                     .color = lv::color::red,
                     .wire_color = lv::color::black});
  viewer.AddObject(&sphere);

  // Drone
  float theta = 0.0F;
  lv::Drone drone;
  viewer.AddObject(&drone);

  // Arrow
  lv::Arrow arrow;
  arrow
      .SetArrowParams({.from_ = Eigen::Vector3d(0.0, 0.0, 0.0),
                       .to_ = Eigen::Vector3d(0.0, 0.0, 5.0),
                       .head_length_ = 1.0,
                       .head_radius_ = 0.2,
                       .body_radius_ = 0.1})
      .SetColor(lv::color::yellow);
  viewer.AddObject(&arrow);

  // Degenerate Indicator
  lv::DegeneracyIndicator degen_indicator;
  degen_indicator.SetDegeneracyInfo({Eigen::Vector3d(1.0, 0.0, 0.0)},
                                    {Eigen::Vector3d(1.0, 0.0, 0.0)});
  viewer.AddObject(&degen_indicator);

  // Odometry
  lv::Odometry odom;
  viewer.AddObject(&odom);

  // Path 3D sin wave
  std::vector<Eigen::Vector3d> path_points;
  for (float x = -10.0F; x <= 10.0F; x += 0.1F) {
    float y = std::sin(x) + 2.0F;
    float z = std::cos(x) + 5.0F;
    path_points.emplace_back(x, y, z);
  }
  lv::Path path;
  path.SetPath(path_points).SetPathWidth(0.1).SetColor(lv::color::rainbow_x);
  viewer.AddObject(&path);

  while (viewer.SpinOnce()) {
    theta += 0.01F;

    drone.SetPos(std::cos(theta) * 6.0, std::sin(theta) * 6.0, 2.0);
    box.SetPos(std::cos(theta + M_PI_4) * 6.0, std::sin(theta + M_PI_4) * 6.0,
               2.0);
    cone.SetPos(std::cos(theta + M_PI_4 * 2) * 6.0,
                std::sin(theta + M_PI_4 * 2) * 6.0, 2.0);
    cylinder.SetPos(std::cos(theta + M_PI_4 * 3) * 6.0,
                    std::sin(theta + M_PI_4 * 3) * 6.0, 2.0);
    sphere.SetPos(std::cos(theta + M_PI_4 * 4) * 6.0,
                  std::sin(theta + M_PI_4 * 4) * 6.0, 2.0);
    arrow.SetFromTo(Eigen::Vector3d(std::cos(theta + M_PI_4 * 4) * 6.0,
                                    std::sin(theta + M_PI_4 * 4) * 6.0, 2.0),
                    Eigen::Vector3d(std::cos(theta + M_PI_4 * 5) * 6.0,
                                    std::sin(theta + M_PI_4 * 5) * 6.0, 2.0));
    degen_indicator.SetPos(Eigen::Vector3d(std::cos(theta + M_PI_4 * 6) * 6.0,
                                           std::sin(theta + M_PI_4 * 6) * 6.0,
                                           2.0));
    odom.SetPos(Eigen::Vector3d(std::cos(theta + M_PI_4 * 7) * 6.0,
                                std::sin(theta + M_PI_4 * 7) * 6.0, 2.0));
  }

  return 0;
}
