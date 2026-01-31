#include "FastLS/FastLS.hpp"
#include "SimScene.hpp"

int main() {
  fastls::FastLS fast_ls{{
      .headless = false,  // Set to true for headless mode
      .vsync = false,     // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
      .fps = true,        // Target frames per second
  }};

  fastls::SimScene sim_scene;
  fast_ls.SetScene(&sim_scene);

  fast_ls.Init();

  while (!(fast_ls.IsQuit())) {
    fast_ls.MainLoop();
  }

  return 0;
}
