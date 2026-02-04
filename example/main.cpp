#include <cmath>

#include "livision/Color.hpp"
#include "livision/MeshBuffer.hpp"
#include "livision/Viewer.hpp"
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

  // Mesh bunny
  auto bunny_res3 =
      std::make_shared<lv::MeshBuffer>("data/bunny/bun_zipper_res3.stl");
  auto bunny_res2 =
      std::make_shared<lv::MeshBuffer>("data/bunny/bun_zipper_res2.stl");
  lv::Mesh bunny(bunny_res3, {.pos = {0.0, 0.0, -2.0},
                              .scale = {50.0, 50.0, 50.0},
                              .color = lv::color::rainbow_z,
                              .wire_color = lv::color::black});
  viewer.AddObject(&bunny.SetDegRotation({90.0, 0.0, 0.0}));

  // Box
  lv::Box box({.scale = {2.0, 2.0, 2.0},
               .color = lv::color::blue,
               .wire_color = lv::color::black});
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

  while (viewer.SpinOnce()) {
    static int counter = 0;
    counter++;
    if (counter % 100 == 0) {
      bunny.SetMeshBuffer(bunny_res2);
    } else if (counter % 100 == 50) {
      bunny.SetMeshBuffer(bunny_res3);
    }

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
  }

  return 0;
}
