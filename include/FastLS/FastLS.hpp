#pragma once
#include <bgfx/bgfx.h>

#include "SceneBase.hpp"

namespace fastls {

struct FastLSConfig {
  bool headless = false;  // Headless mode (no window)
  bool vsync = true;      // Enable VSync
  int width = 1280;       // Window width
  int height = 720;       // Window height
};

class FastLS {
 public:
  explicit FastLS(const FastLSConfig& config)
      : headless_(config.headless),
        vsync_(config.vsync),
        width_(config.width),
        height_(config.height) {
    if (headless_) {
      width_ = 1;
      height_ = 1;
    }
  }

  ~FastLS();

  bool Init();
  void MainLoop();
  void SetScene(SceneBase* scene) {
    scene_ = scene;
    scene_set_ = true;
  }

  bool IsQuit() const { return quit_; }

 private:
  void CameraControl(float* view);
  void PrintFPS();
  static void PrintBackend();

  SDL_Window* window_ = nullptr;
  bgfx::ProgramHandle program_ = BGFX_INVALID_HANDLE;

  // Camera control parameters
  // float cam_pitch_ = 0.0F;
  // float cam_yaw_ = 0.0F;
  // float cam_pan_x_ = 0.0F;
  // float cam_pan_y_ = 0.0F;
  // float zoom_distance_ = 30.0F;

  bx::Vec3 target_ = {0.0F, 0.0F, 5.0F};  // カメラの注視点
  float distance_ = 2.0F;                 // 注視点からの距離（ズーム）
  float cam_yaw_ = 0.0F;                  // ヨー角
  float cam_pitch_ = 0.2F;                // ピッチ角

  // Mouse operation parameters
  float rot_scale_ = 0.01F;
  float pan_scale_ = 0.1F;
  float zoom_scale_ = 0.5F;

  float prev_mouse_x_ = 0;
  float prev_mouse_y_ = 0;

  int width_;
  int height_;

  bool headless_ = false;
  bool vsync_ = true;

  SceneBase* scene_ = nullptr;
  bool scene_set_ = false;

  // FPS計算用の変数
  uint32_t frame_count_ = 0;
  uint64_t last_fps_time_ = 0;
  float current_fps_ = 0.0F;

  bool quit_ = false;
  bool force_visible_ = false;
};
}  // namespace fastls
