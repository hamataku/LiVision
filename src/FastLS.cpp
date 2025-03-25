#include "FastLS/FastLS.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <iostream>

#include "FastLS/SimLidar.hpp"
#include "FastLS/utils.hpp"
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "imgui.h"
#include "sdl-imgui/imgui_impl_sdl3.h"

namespace fastls {

FastLS::~FastLS() {
  // Cleanup
  utils::DeInit();
  bgfx::destroy(program_);

  ImGui_ImplSDL3_Shutdown();
  ImGui_Implbgfx_Shutdown();

  ImGui::DestroyContext();
  bgfx::shutdown();

  SDL_DestroyWindow(window_);
  SDL_Quit();
}

bool FastLS::Init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  if (headless_) {
    window_ = SDL_CreateWindow("Main view", width_, height_,
                               SDL_WINDOW_HIDDEN | SDL_WINDOW_OCCLUDED);
  } else {
    window_ =
        SDL_CreateWindow("Main view", width_, height_, SDL_WINDOW_OCCLUDED);
  }

  if (window_ == nullptr) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    return false;
  }

  bgfx::renderFrame();  // single threaded mode
  bgfx::PlatformData pd{};
#if BX_PLATFORM_WINDOWS
  pd.nwh = SDL_GetPointerProperty(SDL_GetWindowProperties(window_),
                                  SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif BX_PLATFORM_OSX
  pd.nwh =
      SDL_GetPointerProperty(SDL_GetWindowProperties(window_),
                             SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#elif BX_PLATFORM_LINUX
  if (SDL_strcmp(SDL_GetCurrentVideoDriver(), "x11") == 0) {
    pd.ndt =
        SDL_GetPointerProperty(SDL_GetWindowProperties(window_),
                               SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);
    pd.nwh = (void*)static_cast<uintptr_t>(
        SDL_GetNumberProperty(SDL_GetWindowProperties(window_),
                              SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));
  }
#endif

  bgfx::Init bgfx_init;
  bgfx_init.type = bgfx::RendererType::Count;  // auto choose renderer
  bgfx_init.resolution.width = width_;
  bgfx_init.resolution.height = height_;
  if (vsync_) {
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
  } else {
    bgfx_init.resolution.reset = BGFX_RESET_NONE;
  }
  bgfx_init.platformData = pd;
  bgfx::init(bgfx_init);

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0F,
                     0);
  bgfx::setViewRect(0, 0, 0, width_, height_);

  ImGui::CreateContext();

  ImGui_Implbgfx_Init(255);
#if BX_PLATFORM_WINDOWS
  ImGui_ImplSDL3_InitForD3D(window_);
#elif BX_PLATFORM_OSX
  ImGui_ImplSDL3_InitForMetal(window_);
#elif BX_PLATFORM_LINUX
  ImGui_ImplSDL3_InitForVulkan(window_);
#endif

  bool supported = !((bgfx::getCaps()->supported &
                      (BGFX_CAPS_TEXTURE_2D_ARRAY |
                       BGFX_CAPS_TEXTURE_READ_BACK | BGFX_CAPS_COMPUTE)) == 0U);
  if (!supported) {
    std::cerr << "Not supported machine" << std::endl;
    return false;
  }

  const std::string shader_root = "shader/build/";
  bgfx::ShaderHandle vsh =
      utils::CreateShader(shader_root + "v_simple.bin", "vshader");
  bgfx::ShaderHandle fsh =
      utils::CreateShader(shader_root + "f_simple.bin", "fshader");
  program_ = bgfx::createProgram(vsh, fsh, true);

  PrintBackend();
  utils::Init();

  if (scene_ == nullptr) {
    std::cerr << "Scene is not set" << std::endl;
    return false;
  }

  if (scene_set_) {
    scene_->SetHeadless(headless_);
    scene_->Init();
    scene_set_ = false;
  }
  scene_->AddMeshList();
  sim_lidar.Init();

  // FPS計測開始時間の初期化
  last_fps_time_ = SDL_GetTicks();

  return true;
}

void FastLS::MouseOperation() {
  if (!ImGui::GetIO().WantCaptureMouse) {
    float mouse_x;
    float mouse_y;
    const uint32_t buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

    // マウスの移動量を計算
    float delta_x = mouse_x - prev_mouse_x_;
    float delta_y = mouse_y - prev_mouse_y_;

    // 左ボタンドラッグで回転
    if ((buttons & SDL_BUTTON_LMASK) != 0) {
      cam_yaw_ += (delta_x)*rot_scale_;
      cam_pitch_ += (delta_y)*rot_scale_;
    }

    // 中ボタンドラッグでパン
    if ((buttons & SDL_BUTTON_MMASK) != 0) {
      cam_pan_x_ -= delta_x * pan_scale_;
      cam_pan_y_ += delta_y * pan_scale_;  // Y軸は逆方向
    }

    prev_mouse_x_ = mouse_x;
    prev_mouse_y_ = mouse_y;
  }

  // カメラの回転行列を計算
  float cam_rotation[16];
  bx::mtxRotateXYZ(cam_rotation, cam_pitch_, cam_yaw_, 0.0F);

  // パンとズームを含む移動行列を計算
  float cam_translation[16];
  bx::mtxTranslate(cam_translation, cam_pan_x_, cam_pan_y_, zoom_distance_);

  // 最終的なカメラ変換行列を計算
  float cam_transform[16];
  bx::mtxMul(cam_transform, cam_translation, cam_rotation);

  float view[16];
  bx::mtxInverse(view, cam_transform);

  float proj[16];
  bx::mtxProj(proj, 60.0F,
              static_cast<float>(width_) / static_cast<float>(height_), 0.1F,
              100.0F, bgfx::getCaps()->homogeneousDepth, bx::Handedness::Right);

  bgfx::setViewTransform(0, view, proj);
}

void FastLS::MainLoop() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL3_ProcessEvent(&event);
    if (event.type == SDL_EVENT_QUIT) {
      quit_ = true;
      break;
    }
    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
      zoom_distance_ += event.wheel.y * zoom_scale_;
    }
  }

  scene_->UpdateMatrix();

  if (!headless_) {
    scene_->Draw(program_);

    // ImGui_Implbgfx_NewFrame();
    // ImGui_ImplSDL3_NewFrame();

    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow();  // your drawing here
    // ImGui::Render();
    // ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

    MouseOperation();
  }

  scene_->Update();
  bgfx::frame();

  // フレームカウントを増やす
  frame_count_++;

  // 1秒ごとにFPSを表示
  uint64_t current_time = SDL_GetTicks();
  if (current_time - last_fps_time_ >= 1000) {
    PrintFPS();
    // フレームカウントリセット
    frame_count_ = 0;
    last_fps_time_ = current_time;
  }
}

void FastLS::PrintFPS() {
  float elapsed_seconds = (SDL_GetTicks() - last_fps_time_) / 1000.0F;

  if (elapsed_seconds > 0) {
    current_fps_ = frame_count_ / elapsed_seconds;
    std::cout << "FPS: " << current_fps_ << std::endl;
  }
}

void FastLS::PrintBackend() {
  bgfx::RendererType::Enum renderer = bgfx::getRendererType();
  std::cout << "Graphic Backend: ";
  if (renderer == bgfx::RendererType::Vulkan) {
    std::cout << "Vulkan" << std::endl;
  } else if (renderer == bgfx::RendererType::Metal) {
    std::cout << "Metal" << std::endl;
  } else if (renderer == bgfx::RendererType::OpenGL) {
    std::cout << "OpenGL" << std::endl;
  } else {
    std::cout << renderer << std::endl;
  }
}

}  // namespace fastls
