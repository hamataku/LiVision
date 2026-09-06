#pragma once

#include <array>

union SDL_Event;

namespace livision {

/**
 * @brief Per-frame camera input context.
 */
struct CameraInputContext {
  bool want_capture_mouse = false;
  bool want_capture_keyboard = false;
  float delta_time_sec = 0.0F;
};

/**
 * @brief Abstract camera controller interface.
 */
class CameraBase {
 public:
  virtual ~CameraBase() = default;

  /**
   * @brief Consume SDL event input for camera interaction.
   */
  virtual void HandleEvent(const SDL_Event& event) = 0;

  /**
   * @brief Update camera state and return current view matrix.
   */
  virtual const float* Update(const CameraInputContext& context) = 0;
};

/**
 * @brief Mouse-based orbit camera (same behavior as previous Viewer camera).
 */
class MouseOrbitCamera : public CameraBase {
 public:
  MouseOrbitCamera();

  void HandleEvent(const SDL_Event& event) override;
  const float* Update(const CameraInputContext& context) override;

  /**
   * @brief Set the point the camera sits at (this controller flies rather
   * than orbits, so the target is effectively the eye position).
   */
  MouseOrbitCamera* SetTarget(float x, float y, float z);
  /**
   * @brief Set view direction angles in radians.
   */
  MouseOrbitCamera* SetYawPitch(float yaw, float pitch);
  std::array<float, 3> GetTarget() const {
    return {target_x_, target_y_, target_z_};
  }
  float GetYaw() const { return cam_yaw_; }
  float GetPitch() const { return cam_pitch_; }

 protected:
  void UpdateMouse(bool want_capture_mouse);
  void ApplyScroll();
  void RebuildView();

  float view_[16]{};
  float target_x_ = 0.0F;
  float target_y_ = 0.0F;
  float target_z_ = 5.0F;
  float cam_yaw_ = -1.57079632679F;
  float cam_pitch_ = 1.57079632679F - 0.02F;
  float scroll_delta_ = 0.0F;
  int prev_mouse_x_ = 0;
  int prev_mouse_y_ = 0;
};

/**
 * @brief Orbit camera with additional keyboard controls.
 */
class KeyboardOrbitCamera : public MouseOrbitCamera {
 public:
  const float* Update(const CameraInputContext& context) override;

 private:
  void UpdateKeyboard(const CameraInputContext& context);
};

}  // namespace livision
