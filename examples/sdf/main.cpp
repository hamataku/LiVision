#include "livision/Viewer.hpp"
#include "livision/object/Model.hpp"

int main() {
  auto viewer = livision::Viewer::Instance({
      .headless = false,
      .vsync = true,
      .width = 1280,
      .height = 720,
  });

  viewer->RegisterUICallback([&viewer]() {
    if (ImGui::Button("Close")) {
      viewer->Close();
    }
  });

  std::string ex_dir = LIVISION_EXAMPLE_DIR;

  auto sdf_model = livision::Model::InstanceWithFile(
      ex_dir + "/world.sdf",
      {.pos = {0.0, 0.0, 0.0}, .scale = {1.0, 1.0, 1.0}});
  viewer->AddObject(sdf_model);

  while (viewer->SpinOnce()) {
  }

  return 0;
}
