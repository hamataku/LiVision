#include "livision/Viewer.hpp"
#include "livision/marker/Grid.hpp"
#include "livision/object/Model.hpp"
#include "livision/object/primitives.hpp"

int main() {
  livision::Viewer viewer{{
      .headless = false,
      .vsync = true,
      .width = 1280,
      .height = 720,
  }};

  viewer.RegisterUICallback([&viewer]() {
    if (ImGui::Button("Close")) {
      viewer.Close();
    }
  });

  constexpr const char* ex_dir = LIVISION_EXAMPLE_DIR;
  livision::Model sdf_model;
  sdf_model.SetFromFile(std::string(ex_dir) + "/world.sdf");
  sdf_model.SetPos(0.0, 0.0, 0.0);
  sdf_model.SetScale(1.0, 1.0, 1.0);
  viewer.AddObject(&sdf_model);

  while (viewer.SpinOnce()) {
  }

  return 0;
}
