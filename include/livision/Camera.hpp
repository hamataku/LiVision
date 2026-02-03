
void Viewer::CameraControl() {
  // ImGuiがマウスを使用している場合はカメラ操作を無効化
  if (ImGui::GetIO().WantCaptureMouse) {
    SDL_GetMouseState(&prev_mouse_x_, &prev_mouse_y_);
    return;
  }

  int mouse_x;
  int mouse_y;
  const uint32_t buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

  float delta_x = mouse_x - prev_mouse_x_;
  float delta_y = mouse_y - prev_mouse_y_;

  // 回転
  if ((buttons & SDL_BUTTON_LMASK) != 0) {
    cam_yaw_ += delta_x * kRotScale;
    cam_pitch_ -= delta_y * kRotScale;
    cam_pitch_ = bx::clamp(cam_pitch_, -M_PI_2 + 0.01F, M_PI_2 - 0.01F);
  }

  // パン
  if ((buttons & SDL_BUTTON_MMASK) != 0 || (buttons & SDL_BUTTON_RMASK) != 0) {
    float inv_view[16];
    bx::mtxInverse(inv_view, view_);

    bx::Vec3 right = {inv_view[0], inv_view[1], inv_view[2]};
    bx::Vec3 up = {inv_view[4], inv_view[5], inv_view[6]};

    target_ = bx::mad(right, -delta_x * kPanScale, target_);
    target_ = bx::mad(up, delta_y * kPanScale, target_);
  }

  // スクロールで視線方向に target_ を移動
  if (scroll_delta_ != 0.0F) {
    // 現在のカメラ方向を計算
    bx::Vec3 forward = {bx::cos(cam_pitch_) * bx::cos(cam_yaw_),
                        bx::cos(cam_pitch_) * bx::sin(cam_yaw_),
                        bx::sin(cam_pitch_)};

    // 視線方向に target_ を移動
    target_ = bx::mad(forward, scroll_delta_ * kScrollScale, target_);
  }

  prev_mouse_x_ = mouse_x;
  prev_mouse_y_ = mouse_y;

  bx::Vec3 eye = {
      target_.x - (kFixedDistance * bx::cos(cam_pitch_) * bx::cos(cam_yaw_)),
      target_.y - (kFixedDistance * bx::cos(cam_pitch_) * bx::sin(cam_yaw_)),
      target_.z - (kFixedDistance * bx::sin(cam_pitch_))};

  const bx::Vec3 up_vec = {0.0F, 0.0F, 1.0F};
  bx::mtxLookAt(view_, eye, target_, up_vec);
}