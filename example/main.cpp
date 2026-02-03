#include "livision/Color.hpp"
#include "livision/MeshData.hpp"
#include "livision/Viewer.hpp"
#include "livision/object/DroneBody.hpp"
#include "livision/object/Mesh.hpp"
#include "livision/object/primitives.hpp"

int main() {
  livision::Viewer fast_ls{{
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
      .fps = true,        // Target frames per second
  }};

  // Plane
  livision::Plane plane(
      {.scale = {40.0, 40.0, 0.0}, .color = livision::color::light_gray});
  fast_ls.AddObject(&plane);

  // Mesh bunny
  auto bunny_res3 =
      std::make_shared<livision::MeshData>("data/bunny/bun_zipper_res3.stl");
  auto bunny_res2 =
      std::make_shared<livision::MeshData>("data/bunny/bun_zipper_res2.stl");

  livision::Mesh bunny(bunny_res3, {.pos = {0.0, 0.0, -2.0},
                                    .scale = {50.0, 50.0, 50.0},
                                    .color = livision::color::rainbow_z});
  fast_ls.AddObject(&bunny.SetDegRotation({90.0, 0.0, 0.0}));

  // Drone body
  float theta = 0.0F;
  livision::DroneBody drone;
  fast_ls.AddObject(&drone);

  while (fast_ls.SpinOnce()) {
    static int counter = 0;
    counter++;
    if (counter % 100 == 0) {
      bunny.SetMeshData(bunny_res2);
    } else if (counter % 100 == 50) {
      bunny.SetMeshData(bunny_res3);
    }

    theta += 0.01F;
    drone.SetPos(std::cos(theta) * 6.0, std::sin(theta) * 6.0, 2.0);
  }

  return 0;
}
