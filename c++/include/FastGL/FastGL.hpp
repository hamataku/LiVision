#pragma once
#include <SDL3/SDL.h>
#include <bgfx/bgfx.h>

#include <string>

#include "SceneBase.hpp"

namespace fastgl {

class FastGL {
 public:
  bool Run(bool headless = false);
  void SetScene(SceneBase* scene) {
    scene_ = scene;
    scene_set_ = true;
  }

 private:
  bool Init();
  void MouseOperation();
  void MainLoop();
  void PrintFPS();
  static void PrintBackend();
  static bgfx::ShaderHandle CreateShader(const std::string& shader,
                                         const char* name);

  SDL_Window* window_ = nullptr;
  bgfx::ProgramHandle program_ = BGFX_INVALID_HANDLE;

  // Camera control parameters
  float cam_pitch_ = 0.0F;
  float cam_yaw_ = 0.0F;
  float cam_pan_x_ = 3.0F;
  float cam_pan_y_ = 3.0F;
  float zoom_distance_ = -6.0F;

  // Mouse operation parameters
  float rot_scale_ = 0.01F;
  float pan_scale_ = 0.01F;
  float zoom_scale_ = 0.5F;
  float prev_mouse_x_ = 0;
  float prev_mouse_y_ = 0;

  int width_ = 800;
  int height_ = 600;

  bool headless_ = false;

  SceneBase* scene_ = nullptr;
  bool scene_set_ = false;

  Uint64 last_counter_ = 0;
  int fps_print_counter_ = 0;
};
}  // namespace fastgl
