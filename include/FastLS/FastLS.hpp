#pragma once
#include <bgfx/bgfx.h>

#include <cmath>

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

  bool Init();
  void Exit();
  void MainLoop();
  void SetScene(SceneBase* scene) {
    scene_ = scene;
    scene_set_ = true;
  }

  bool IsQuit() const { return quit_; }

 private:
  void CameraControl();
  void PrintFPS();
  static void PrintBackend();

  SDL_Window* window_ = nullptr;
  bgfx::ProgramHandle program_ = BGFX_INVALID_HANDLE;
  float view_[16];
  float proj_[16];

  bx::Vec3 target_ = {0.0F, 0.0F, 30.0F};  // カメラの注視点
  float cam_yaw_ = -M_PI_2;                // ヨー角
  float cam_pitch_ = M_PI_2 - 0.02F;       // ピッチ角

  // Mouse operation parameters
  static constexpr float kRotScale = 0.002F;
  static constexpr float kPanScale = 0.01F;
  static constexpr float kScrollScale = 1.2F;
  static constexpr float kFixedDistance = 0.1F;

  float scroll_delta_ = 0.0F;  // スクロールの移動量
  float prev_mouse_x_ = 0;
  float prev_mouse_y_ = 0;

  bool headless_ = false;
  bool vsync_ = true;

  int width_;
  int height_;

  SceneBase* scene_ = nullptr;
  bool scene_set_ = false;

  // FPS計算用の変数
  uint32_t frame_count_ = 0;
  uint64_t last_fps_time_ = 0;
  float current_fps_ = 0.0F;

  bool quit_ = false;
};
}  // namespace fastls
