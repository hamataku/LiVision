#include "livision/Color.hpp"
#include "livision/MeshBuffer.hpp"
#include "livision/Viewer.hpp"
#include "livision/object/Drone.hpp"
#include "livision/object/Mesh.hpp"
#include "livision/object/primitives.hpp"

int main() {
  using namespace livision;
  Viewer viewer{{
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
      .fps = true,        // Target frames per second
  }};

  // Plane
  Plane plane({.scale = {40.0, 40.0, 0.0}, .color = color::light_gray});
  viewer.AddObject(&plane);

  // Mesh bunny
  auto bunny_res3 =
      std::make_shared<livision::MeshBuffer>("data/bunny/bun_zipper_res3.stl");
  auto bunny_res2 =
      std::make_shared<livision::MeshBuffer>("data/bunny/bun_zipper_res2.stl");

  Mesh bunny(bunny_res3, {.pos = {0.0, 0.0, -2.0},
                          .scale = {50.0, 50.0, 50.0},
                          .color = color::rainbow_z,
                          .wire_color = color::black});
  viewer.AddObject(&bunny.SetDegRotation({90.0, 0.0, 0.0}));

  // Drone
  float theta = 0.0F;
  Drone drone;
  viewer.AddObject(&drone);

  // Sphere
  Sphere sphere({.pos = {0.0, 4.0, 1.0},
                 .scale = {2.0, 2.0, 2.0},
                 .color = color::red,
                 .wire_color = color::black});
  viewer.AddObject(&sphere);

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
  }

  return 0;
}
