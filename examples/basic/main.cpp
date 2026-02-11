#include <cmath>

#include "livision/Color.hpp"
#include "livision/Viewer.hpp"
// objects
#include "livision/marker/Arrow.hpp"
#include "livision/marker/DegeneracyIndicator.hpp"
#include "livision/marker/Grid.hpp"
#include "livision/marker/Odometry.hpp"
#include "livision/marker/Path.hpp"
#include "livision/object/Drone.hpp"
#include "livision/object/Model.hpp"
#include "livision/object/Text.hpp"
#include "livision/object/primitives.hpp"

int main() {
  livision::Viewer viewer{{
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
  }};

  // UI callback
  viewer.RegisterUICallback([&viewer]() {
    if (ImGui::Button("Close")) {
      viewer.Close();
    }

    ImGuiIO& io = ImGui::GetIO();
    if (!io.WantCaptureKeyboard) {
      if (ImGui::IsKeyDown(ImGuiKey_Q)) {
        viewer.Close();
      }
    }
  });

  // Plane
  auto plane = livision::Plane::Instance(
      {.scale = {40.0, 40.0, 0.0}, .color = livision::color::light_gray});
  viewer.AddObject(plane);

  // Grid
  auto grid = livision::Grid::Instance();
  grid->SetResolution(1.0).SetScale(Eigen::Vector3d(40.0, 40.0, 0.0));
  viewer.AddObject(grid);

  // Mesh bunny
  constexpr const char* ex_dir = LIVISION_EXAMPLE_DIR;
  auto bunny = livision::Model::InstanceWithFile(
      std::string(ex_dir) + "/bunny.stl",
      {
          .pos = {0.0, 0.0, -2.0},
          .scale = {50.0, 50.0, 50.0},
          .color = livision::color::rainbow_z,
          .wire_color = livision::color::black,
      });
  bunny->SetDegRotation({90.0, 0.0, 0.0});
  viewer.AddObject(bunny);

  // Box
  auto box = livision::Box::Instance(
      {.scale = {2.0, 2.0, 2.0}, .color = livision::color::rainbow_z});
  viewer.AddObject(box);

  // Cone
  auto cone = livision::Cone::Instance({.scale = {1.5, 1.5, 3.0},
                                        .color = livision::color::green,
                                        .wire_color = livision::color::black});
  viewer.AddObject(cone);

  // Cylinder
  auto cylinder =
      livision::Cylinder::Instance({.scale = {1.5, 1.5, 3.0},
                                    .color = livision::color::orange,
                                    .wire_color = livision::color::black});
  viewer.AddObject(cylinder);

  // Sphere
  auto sphere =
      livision::Sphere::Instance({.scale = {2.0, 2.0, 2.0},
                                  .color = livision::color::red,
                                  .wire_color = livision::color::black});
  viewer.AddObject(sphere);

  // Drone
  float theta = 0.0F;
  auto drone = livision::Drone::Instance();
  // Attach 3D text
  auto text = livision::Text::Instance();
  text->SetText("Drone");
  text->SetPos(0.0, 0.0, 0.8);
  text->SetColor(livision::color::red);
  text->SetHeight(0.5F);
  text->SetAlign(livision::TextAlign::Center);
  drone->AddObject(text);
  viewer.AddObject(drone);

  // Arrow
  auto arrow = livision::Arrow::Instance();
  arrow
      ->SetArrowParams({.from_ = Eigen::Vector3d(0.0, 0.0, 0.0),
                        .to_ = Eigen::Vector3d(0.0, 0.0, 5.0),
                        .head_length_ = 1.0,
                        .head_radius_ = 0.2,
                        .body_radius_ = 0.1})
      .SetColor(livision::color::yellow);
  viewer.AddObject(arrow);

  // Degenerate Indicator
  auto degen_indicator = livision::DegeneracyIndicator::Instance();
  degen_indicator->SetDegeneracyInfo({Eigen::Vector3d(1.0, 0.0, 0.0)},
                                     {Eigen::Vector3d(1.0, 0.0, 0.0)});
  viewer.AddObject(degen_indicator);

  // Odometry
  auto odom = livision::Odometry::Instance();
  viewer.AddObject(odom);

  // Path 3D sin wave
  std::vector<Eigen::Vector3d> path_points;
  for (float x = -10.0F; x <= 10.0F; x += 0.1F) {
    float y = std::sin(x) + 2.0F;
    float z = std::cos(x) + 5.0F;
    path_points.emplace_back(x, y, z);
  }
  auto path = livision::Path::Instance();
  path->SetPath(path_points)
      .SetPathWidth(0.1)
      .SetColor(livision::color::rainbow_x);
  viewer.AddObject(path);

  while (viewer.SpinOnce()) {
    theta += 0.01F;

    drone->SetPos(std::cos(theta) * 6.0, std::sin(theta) * 6.0, 2.0);
    box->SetPos(std::cos(theta + M_PI_4) * 6.0, std::sin(theta + M_PI_4) * 6.0,
                2.0);
    cone->SetPos(std::cos(theta + M_PI_4 * 2) * 6.0,
                 std::sin(theta + M_PI_4 * 2) * 6.0, 2.0);
    cylinder->SetPos(std::cos(theta + M_PI_4 * 3) * 6.0,
                     std::sin(theta + M_PI_4 * 3) * 6.0, 2.0);
    sphere->SetPos(std::cos(theta + M_PI_4 * 4) * 6.0,
                   std::sin(theta + M_PI_4 * 4) * 6.0, 2.0);
    arrow->SetFromTo(Eigen::Vector3d(std::cos(theta + M_PI_4 * 4) * 6.0,
                                     std::sin(theta + M_PI_4 * 4) * 6.0, 2.0),
                     Eigen::Vector3d(std::cos(theta + M_PI_4 * 5) * 6.0,
                                     std::sin(theta + M_PI_4 * 5) * 6.0, 2.0));
    degen_indicator->SetPos(Eigen::Vector3d(std::cos(theta + M_PI_4 * 6) * 6.0,
                                            std::sin(theta + M_PI_4 * 6) * 6.0,
                                            2.0));
    odom->SetPos(Eigen::Vector3d(std::cos(theta + M_PI_4 * 7) * 6.0,
                                 std::sin(theta + M_PI_4 * 7) * 6.0, 2.0));
  }

  return 0;
}
