#include "livision/LiVision.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <cstdlib>
#include <iostream>

#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "imgui.cmake/imgui/backends/imgui_impl_sdl2.h"
#include "imgui.h"
#include "implot/implot.h"
#include "livision/Container.hpp"
#include "livision/sim/LidarSim.hpp"
#include "livision/utils.hpp"

namespace livision {
LiVision::LiVision(const LiVisionConfig& config)
    : headless_(config.headless),
      vsync_(config.vsync),
      width_(config.width),
      height_(config.height),
      fps_(config.fps) {
  if (headless_) {
    width_ = 1;
    height_ = 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
    std::exit(EXIT_FAILURE);
  }

  if (headless_) {
    window_ = SDL_CreateWindow("Main view", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, width_, height_,
                               SDL_WINDOW_HIDDEN);
  } else {
    window_ = SDL_CreateWindow("Main view", SDL_WINDOWPOS_UNDEFINED,
                               SDL_WINDOWPOS_UNDEFINED, width_, height_, 0);
  }

  if (window_ == nullptr) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    std::exit(EXIT_FAILURE);
  }

  bgfx::renderFrame();  // single threaded mode
  bgfx::PlatformData pd{};
  SDL_SysWMinfo wm_info;
  SDL_VERSION(&wm_info.version);
  if (SDL_GetWindowWMInfo(window_, &wm_info)) {
    pd.ndt = wm_info.info.x11.display;  // Display*
#if BX_PLATFORM_WINDOWS
    pd.nwh = wm_info.info.win.window;  // HWND
#elif BX_PLATFORM_OSX
    pd.nwh = wm_info.info.cocoa.window;  // NSWindow*
#elif BX_PLATFORM_LINUX
    pd.nwh =
        (void*)static_cast<uintptr_t>(wm_info.info.x11.window);  // Window (XID)
#endif
  }

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
  ImPlot::CreateContext();

  ImGui_Implbgfx_Init(255);
#if BX_PLATFORM_WINDOWS
  ImGui_ImplSDL2_InitForD3D(window_);
#elif BX_PLATFORM_OSX
  ImGui_ImplSDL2_InitForMetal(window_);
#elif BX_PLATFORM_LINUX
  ImGui_ImplSDL2_InitForVulkan(window_);
#endif

  const bgfx::Caps* caps = bgfx::getCaps();

  bool supported = !((caps->supported &
                      (BGFX_CAPS_TEXTURE_2D_ARRAY |
                       BGFX_CAPS_TEXTURE_READ_BACK | BGFX_CAPS_COMPUTE)) == 0U);
  if (!supported) {
    std::cerr << "Not supported machine" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  const std::string shader_root = "shader/bin/";
#if BX_PLATFORM_WINDOWS
  const std::string plt_name = "win";
#elif BX_PLATFORM_OSX
  const std::string plt_name = "mac";
#elif BX_PLATFORM_LINUX
  const std::string plt_name = "linux";
#endif

  bgfx::ShaderHandle vsh = utils::CreateShader(
      shader_root + "v_simple_" + plt_name + ".bin", "vshader");
  bgfx::ShaderHandle fsh = utils::CreateShader(
      shader_root + "f_simple_" + plt_name + ".bin", "fshader");
  program_ = bgfx::createProgram(vsh, fsh, true);

  bgfx::ShaderHandle vph = utils::CreateShader(
      shader_root + "v_points_" + plt_name + ".bin", "vshader_points");
  bgfx::ShaderHandle fph = utils::CreateShader(
      shader_root + "f_points_" + plt_name + ".bin", "fshader_points");
  if (bgfx::isValid(vph) && bgfx::isValid(fph)) {
    utils::point_program = bgfx::createProgram(vph, fph, true);
  }

  PrintBackend();
  utils::Init();
}

LiVision::~LiVision() {
  // Cleanup
  utils::DeInit();
  bgfx::destroy(program_);
  if (bgfx::isValid(utils::point_program)) {
    bgfx::destroy(utils::point_program);
    utils::point_program = BGFX_INVALID_HANDLE;
  }
  lidar_sim.Destroy();

  ImGui_ImplSDL2_Shutdown();
  ImGui_Implbgfx_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  bgfx::shutdown();

  SDL_DestroyWindow(window_);
  SDL_Quit();
  std::cout << "### LiVision Exit ###" << std::endl;
}

void LiVision::CameraControl() {
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

bool LiVision::SpinOnce() {
  if (!initialized_) {
    InitMeshList();
    lidar_sim.Init();

    // FPS計測開始時間の初期化
    last_fps_time_ = SDL_GetTicks();
    initialized_ = true;
  }

  UpdateMatrix();

  if (!headless_) {
    // Camera control
    Draw(program_);

    bx::mtxProj(proj_, 60.0F,
                static_cast<float>(width_) / static_cast<float>(height_), 0.1F,
                1000.0F, bgfx::getCaps()->homogeneousDepth,
                bx::Handedness::Right);

    CameraControl();
    bgfx::setViewTransform(0, view_, proj_);

    // Event handling
    SDL_Event event = {};
    scroll_delta_ = 0.0F;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        quit_ = true;
      }
      if (event.type == SDL_MOUSEWHEEL) {
        scroll_delta_ = event.wheel.y;
      }
      // scene_->EventHandler(event);
    }

    // Render ImGui
    ImGui_Implbgfx_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();

    // ImGui::Begin("Control panel", nullptr,
    // ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Begin("Control panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    // scene_->GuiCustomize();

    ImGui::End();
    ImGui::Render();
    ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
  } else {
    // Headless Event handling(Catch Ctrl+C)
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        quit_ = true;
      }
    }
  }

  UpdateDynamicMeshList();
  lidar_sim.CalcPointCloud();

  // フレームカウントを増やす
  frame_count_++;

  // 1秒ごとにFPSを表示
  uint64_t current_time = SDL_GetTicks();
  if (current_time - last_fps_time_ >= 1000) {
    PrintFPS();
    frame_count_ = 0;
    last_fps_time_ = current_time;
  }

  return !quit_;
}

void LiVision::AddObject(ObjectBase* object) {
  object->Init();
  auto* container = dynamic_cast<Container*>(object);
  if (container) {
    for (auto* obj : container->GetObjects()) {
      obj->Init();
      AddObject(obj);
    }
  } else {
    objects_.push_back(object);
  }
}

void LiVision::InitMeshList() {
  for (auto* object : objects_) {
    object->UpdateMatrix();
  }
  for (auto* object : objects_) {
    if (object->IsLidarVisible()) {
      lidar_sim.InitMeshList(object);
    }
  }
}

void LiVision::UpdateMatrix() {
  for (auto* object : objects_) {
    object->UpdateMatrix();
  }
}

void LiVision::UpdateDynamicMeshList() {
  for (auto* object : objects_) {
    if (object->IsLidarVisible()) {
      lidar_sim.UpdateDynamicMeshList(object);
    }
  }
}

void LiVision::Draw(bgfx::ProgramHandle& program) {
  for (auto* object : objects_) {
    if (object->IsVisible()) object->Draw(program);
  }
}

void LiVision::PrintFPS() {
  float elapsed_seconds = (SDL_GetTicks() - last_fps_time_) / 1000.0F;

  if (fps_ && elapsed_seconds > 0) {
    current_fps_ = frame_count_ / elapsed_seconds;
    std::cout << "FPS: " << current_fps_ << std::endl;
  }
}

void LiVision::PrintBackend() {
  const bgfx::Caps* caps = bgfx::getCaps();

  // ベンダーIDからベンダー名を特定
  const char* vendor = "Unknown";
  if (caps->vendorId == BGFX_PCI_ID_AMD) {
    vendor = "AMD";
  } else if (caps->vendorId == BGFX_PCI_ID_INTEL) {
    vendor = "Intel";
  } else if (caps->vendorId == BGFX_PCI_ID_NVIDIA) {
    vendor = "NVIDIA";
  } else if (caps->vendorId == BGFX_PCI_ID_MICROSOFT) {
    vendor = "Microsoft";
  } else if (caps->vendorId == BGFX_PCI_ID_ARM) {
    vendor = "ARM";
  } else if (caps->vendorId == BGFX_PCI_ID_APPLE) {
    vendor = "Apple";
  } else if (caps->vendorId == BGFX_PCI_ID_SOFTWARE_RASTERIZER) {
    vendor = "Software Rasterizer";
  }

  printf("- Vendor: %s (ID: 0x%04x)\n", vendor, caps->vendorId);
  printf("- Device ID: 0x%04x\n", caps->deviceId);

  std::cout << "- Graphic Backend: "
            << bgfx::getRendererName(bgfx::getRendererType()) << std::endl;
}

}  // namespace livision
