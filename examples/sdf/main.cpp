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

  auto sdf_model = livision::Model::InstanceWithPath(
      ex_dir + "/world.sdf",
      {.pos = {0.0, 0.0, 0.0}, .scale = {1.0, 1.0, 1.0}});
  viewer->AddObject(sdf_model);

  sdf_model->PrintTree();
  auto rotor_0 = sdf_model->GetContainerByPath("ht01/rotor_0");
  auto rotor_1 = sdf_model->GetContainerByPath("ht01/rotor_1");
  auto rotor_2 = sdf_model->GetContainerByPath("ht01/rotor_2");
  auto rotor_3 = sdf_model->GetContainerByPath("ht01/rotor_3");

  float angle = 0.0F;
  while (viewer->SpinOnce()) {
    angle += 20.0F;
    if (angle >= 360.0F) {
      angle -= 360.0F;
    }
    rotor_0->SetDegRotation({0.0F, 0.0F, angle});
    rotor_1->SetDegRotation({0.0F, 0.0F, angle});
    rotor_2->SetDegRotation({0.0F, 0.0F, -angle});
    rotor_3->SetDegRotation({0.0F, 0.0F, -angle});
  }

  return 0;
}
