#include "FastLS/FastLS.hpp"
#include "SimScene.hpp"

int main() {
  fastls::FastLS fast_ls{false, true};
  // fastls::FastLS fast_ls{true, false};

  fastls::SimScene sim_scene;
  fast_ls.SetScene(&sim_scene);

  fast_ls.Run();
  return 0;
}
