#include "livision/Camera.hpp"

#include <SDL.h>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <bx/math.h>

namespace livision {

namespace {
constexpr float kRotScale = 0.002F;
constexpr float kPanScale = 0.02F;
constexpr float kScrollScale = 1.2F;
constexpr float kFixedDistance = 0.1F;

constexpr float kKeyboardMoveSpeed = 2.5F;
constexpr float kKeyboardRotSpeed = 2.0F;
}  // namespace

MouseOrbitCamera::MouseOrbitCamera() { RebuildView(); }

void MouseOrbitCamera::HandleEvent(const SDL_Event& event) {
  if (event.type == SDL_MOUSEWHEEL) {
    scroll_delta_ += static_cast<float>(event.wheel.y);
  }
}

const float* MouseOrbitCamera::Update(const CameraInputContext& context) {
  UpdateMouse(context.want_capture_mouse);
  ApplyScroll();
  RebuildView();
  return view_;
}

void MouseOrbitCamera::UpdateMouse(bool want_capture_mouse) {
  if (want_capture_mouse) {
    SDL_GetMouseState(&prev_mouse_x_, &prev_mouse_y_);
    return;
  }

  int mouse_x = 0;
  int mouse_y = 0;
  const uint32_t buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

  const int delta_x = mouse_x - prev_mouse_x_;
  const int delta_y = mouse_y - prev_mouse_y_;

  if ((buttons & SDL_BUTTON_LMASK) != 0) {
    cam_yaw_ += static_cast<float>(delta_x) * kRotScale;
    cam_pitch_ -= static_cast<float>(delta_y) * kRotScale;
    cam_pitch_ = bx::clamp(cam_pitch_, -bx::kPiHalf + 0.01F, bx::kPiHalf - 0.01F);
  }

  if ((buttons & SDL_BUTTON_MMASK) != 0 || (buttons & SDL_BUTTON_RMASK) != 0) {
    float inv_view[16];
    bx::mtxInverse(inv_view, view_);

    const bx::Vec3 right = {inv_view[0], inv_view[1], inv_view[2]};
    const bx::Vec3 up = {inv_view[4], inv_view[5], inv_view[6]};

    const bx::Vec3 target = {target_x_, target_y_, target_z_};
    const bx::Vec3 panned_x =
        bx::mad(right, -static_cast<float>(delta_x) * kPanScale, target);
    const bx::Vec3 panned =
        bx::mad(up, static_cast<float>(delta_y) * kPanScale, panned_x);
    target_x_ = panned.x;
    target_y_ = panned.y;
    target_z_ = panned.z;
  }

  prev_mouse_x_ = mouse_x;
  prev_mouse_y_ = mouse_y;
}

void MouseOrbitCamera::ApplyScroll() {
  if (scroll_delta_ == 0.0F) {
    return;
  }

  const bx::Vec3 forward = {bx::cos(cam_pitch_) * bx::cos(cam_yaw_),
                            bx::cos(cam_pitch_) * bx::sin(cam_yaw_),
                            bx::sin(cam_pitch_)};
  const bx::Vec3 target = {target_x_, target_y_, target_z_};
  const bx::Vec3 moved = bx::mad(forward, scroll_delta_ * kScrollScale, target);
  target_x_ = moved.x;
  target_y_ = moved.y;
  target_z_ = moved.z;
  scroll_delta_ = 0.0F;
}

void MouseOrbitCamera::RebuildView() {
  const bx::Vec3 eye = {
      target_x_ - (kFixedDistance * bx::cos(cam_pitch_) * bx::cos(cam_yaw_)),
      target_y_ - (kFixedDistance * bx::cos(cam_pitch_) * bx::sin(cam_yaw_)),
      target_z_ - (kFixedDistance * bx::sin(cam_pitch_))};

  const bx::Vec3 up_vec = {0.0F, 0.0F, 1.0F};
  const bx::Vec3 target = {target_x_, target_y_, target_z_};
  bx::mtxLookAt(view_, eye, target, up_vec);
}

const float* KeyboardOrbitCamera::Update(const CameraInputContext& context) {
  UpdateMouse(context.want_capture_mouse);
  UpdateKeyboard(context);
  ApplyScroll();
  RebuildView();
  return view_;
}

void KeyboardOrbitCamera::UpdateKeyboard(const CameraInputContext& context) {
  if (context.want_capture_keyboard || context.delta_time_sec <= 0.0F) {
    return;
  }

  const uint8_t* key_state = SDL_GetKeyboardState(nullptr);
  if (key_state == nullptr) {
    return;
  }

  const float move_scale = kKeyboardMoveSpeed * context.delta_time_sec;
  const float rot_scale = kKeyboardRotSpeed * context.delta_time_sec;

  const bx::Vec3 forward = {bx::cos(cam_pitch_) * bx::cos(cam_yaw_),
                            bx::cos(cam_pitch_) * bx::sin(cam_yaw_),
                            bx::sin(cam_pitch_)};
  const bx::Vec3 world_up = {0.0F, 0.0F, 1.0F};
  const bx::Vec3 right = bx::normalize(bx::cross(forward, world_up));

  if (key_state[SDL_SCANCODE_W] != 0) {
    const bx::Vec3 target = {target_x_, target_y_, target_z_};
    const bx::Vec3 moved = bx::mad(forward, move_scale, target);
    target_x_ = moved.x;
    target_y_ = moved.y;
    target_z_ = moved.z;
  }
  if (key_state[SDL_SCANCODE_S] != 0) {
    const bx::Vec3 target = {target_x_, target_y_, target_z_};
    const bx::Vec3 moved = bx::mad(forward, -move_scale, target);
    target_x_ = moved.x;
    target_y_ = moved.y;
    target_z_ = moved.z;
  }
  if (key_state[SDL_SCANCODE_A] != 0) {
    const bx::Vec3 target = {target_x_, target_y_, target_z_};
    const bx::Vec3 moved = bx::mad(right, -move_scale, target);
    target_x_ = moved.x;
    target_y_ = moved.y;
    target_z_ = moved.z;
  }
  if (key_state[SDL_SCANCODE_D] != 0) {
    const bx::Vec3 target = {target_x_, target_y_, target_z_};
    const bx::Vec3 moved = bx::mad(right, move_scale, target);
    target_x_ = moved.x;
    target_y_ = moved.y;
    target_z_ = moved.z;
  }
  if (key_state[SDL_SCANCODE_Q] != 0) {
    target_z_ -= move_scale;
  }
  if (key_state[SDL_SCANCODE_E] != 0) {
    target_z_ += move_scale;
  }

  if (key_state[SDL_SCANCODE_LEFT] != 0) {
    cam_yaw_ -= rot_scale;
  }
  if (key_state[SDL_SCANCODE_RIGHT] != 0) {
    cam_yaw_ += rot_scale;
  }
  if (key_state[SDL_SCANCODE_UP] != 0) {
    cam_pitch_ += rot_scale;
  }
  if (key_state[SDL_SCANCODE_DOWN] != 0) {
    cam_pitch_ -= rot_scale;
  }

  cam_pitch_ = bx::clamp(cam_pitch_, -bx::kPiHalf + 0.01F, bx::kPiHalf - 0.01F);
}

}  // namespace livision
