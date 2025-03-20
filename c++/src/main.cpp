#include "FastGL/FastGL.hpp"
#include "SimScene.hpp"

int main() {
  fastgl::FastGL fast_gl;

  SimScene sim_scene;
  fast_gl.SetScene(&sim_scene);

  fast_gl.Run(true);
  return 0;
}
