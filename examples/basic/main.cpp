#include "livision/Viewer.hpp"
#include "livision/marker/Grid.hpp"
#include "livision/object/primitives.hpp"

int main() {
  auto viewer = livision::Viewer::Instance({
      .width = 1280,
      .height = 720,
  });

  // UI callback
  float theta = 0;
  viewer->RegisterUICallback([&]() {
    ImGui::SliderFloat("theta", &theta, 0, M_PI * 2.0F);
    if (ImGui::Button("Close")) {
      viewer->Close();
    }
  });

  // Grid
  auto grid = livision::Grid::Instance({.scale = {15.0, 15.0, 0.0}});
  viewer->AddObject(grid);

  // Sphere
  auto sphere =
      livision::Sphere::Instance({.pos = {0.0, 0.0, 0.0},
                                  .scale = {2.0, 2.0, 2.0},
                                  .color = livision::color::rainbow_z,
                                  .wire_color = livision::color::black});
  viewer->AddObject(sphere);

  while (viewer->SpinOnce()) {
    sphere->SetRadRotation({0, 0, theta});
  }
  return 0;
}