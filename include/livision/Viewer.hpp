#pragma once
#include <memory>

#include "livision/ObjectBase.hpp"

namespace livision {

struct ViewerConfig {
  bool headless = false;  // Headless mode (no window)
  bool vsync = true;      // Enable VSync
  int width = 1280;       // Window width
  int height = 720;       // Window height
  bool fps = false;       // Display FPS
};

class Viewer {
 public:
  explicit Viewer(const ViewerConfig& config);
  ~Viewer();

  bool SpinOnce();
  void AddObject(ObjectBase* object);

 private:
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision

// class Viewer {
//  public:
//   explicit Viewer(const ViewerConfig& config);
//   ~Viewer();

//   bool SpinOnce();
//   void AddObject(ObjectBase* object);

//  private:
//   void InitMeshList();
//   void UpdateMatrix();
//   void UpdateDynamicMeshList();
//   void Draw(bgfx::ProgramHandle& program);
//   void CameraControl();

//   void PrintFPS();
//   static void PrintBackend();

//   std::vector<ObjectBase*> objects_;

//   SDL_Window* window_ = nullptr;
//   bgfx::ProgramHandle program_ = BGFX_INVALID_HANDLE;
//   float view_[16];
//   float proj_[16];

//   bx::Vec3 target_ = {0.0F, 0.0F, 5.0F};  // カメラの注視点
//   float cam_yaw_ = -M_PI_2;               // ヨー角
//   float cam_pitch_ = M_PI_2 - 0.02F;      // ピッチ角

//   // Mouse operation parameters
//   static constexpr float kRotScale = 0.002F;
//   static constexpr float kPanScale = 0.02F;
//   static constexpr float kScrollScale = 1.2F;
//   static constexpr float kFixedDistance = 0.1F;

//   float scroll_delta_ = 0.0F;  // スクロールの移動量
//   int prev_mouse_x_ = 0;
//   int prev_mouse_y_ = 0;

//   bool headless_ = false;
//   bool vsync_ = true;

//   int width_;
//   int height_;
//   bool fps_;

//   bool initialized_ = false;

//   // FPS計算用の変数
//   uint32_t frame_count_ = 0;
//   uint64_t last_fps_time_ = 0;
//   float current_fps_ = 0.0F;

//   bool quit_ = false;
// };
// }  // namespace livision
