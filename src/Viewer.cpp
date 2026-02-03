#include "livision/Viewer.hpp"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <iostream>

#include "imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui_impl_bgfx.h"
#include "implot.h"
#include "livision/Container.hpp"
#include "livision/Renderer.hpp"

namespace livision {

static constexpr float kRotScale = 0.002F;
static constexpr float kPanScale = 0.02F;
static constexpr float kScrollScale = 1.2F;
static constexpr float kFixedDistance = 0.1F;

struct Viewer::Impl {
  SDL_Window* window = nullptr;
  std::vector<ObjectBase*> objects;
  ViewerConfig config;
  Renderer renderer;

  bool initialized = false;
  bool quit = false;

  int frame_count = 0;
  uint32_t last_fps_time = 0;

  float scroll_delta = 0.0F;  // スクロールの移動量
  int prev_mouse_x = 0;
  int prev_mouse_y = 0;
  float view[16];
  float proj[16];
  bx::Vec3 target = {0.0F, 0.0F, 5.0F};  // カメラの注視点
  float cam_yaw = -M_PI_2;               // ヨー角
  float cam_pitch = M_PI_2 - 0.02F;      // ピッチ角
};

Viewer::Viewer(const ViewerConfig& config) : pimpl_(std::make_unique<Impl>()) {
  pimpl_->config = config;

  if (pimpl_->config.headless) {
    pimpl_->config.width = 1;
    pimpl_->config.height = 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("SDL could not initialize. SDL_Error: %s\n", SDL_GetError());
    std::exit(EXIT_FAILURE);
  }

  if (pimpl_->config.headless) {
    pimpl_->window = SDL_CreateWindow(
        "Main view", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        pimpl_->config.width, pimpl_->config.height, SDL_WINDOW_HIDDEN);
  } else {
    pimpl_->window = SDL_CreateWindow(
        "Main view", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        pimpl_->config.width, pimpl_->config.height, 0);
  }

  if (pimpl_->window == nullptr) {
    printf("Window could not be created. SDL_Error: %s\n", SDL_GetError());
    std::exit(EXIT_FAILURE);
  }

  bgfx::renderFrame();  // single threaded mode
  bgfx::PlatformData pd{};
  SDL_SysWMinfo wm_info;
  SDL_VERSION(&wm_info.version);
  if (SDL_GetWindowWMInfo(pimpl_->window, &wm_info)) {
#if BX_PLATFORM_WINDOWS
    pd.nwh = wm_info.info.win.window;  // HWND
#elif BX_PLATFORM_OSX
    pd.nwh = wm_info.info.cocoa.window;  // NSWindow*
#elif BX_PLATFORM_LINUX
    pd.ndt = wm_info.info.x11.display;  // Display*
    pd.nwh =
        (void*)static_cast<uintptr_t>(wm_info.info.x11.window);  // Window (XID)
#endif
  }

  bgfx::Init bgfx_init;
  bgfx_init.type = bgfx::RendererType::Count;  // auto choose renderer
  bgfx_init.resolution.width = pimpl_->config.width;
  bgfx_init.resolution.height = pimpl_->config.height;
  if (pimpl_->config.vsync) {
    bgfx_init.resolution.reset = BGFX_RESET_VSYNC;
  } else {
    bgfx_init.resolution.reset = BGFX_RESET_NONE;
  }
  bgfx_init.platformData = pd;
  bgfx::init(bgfx_init);

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0F,
                     0);
  bgfx::setViewRect(0, 0, 0, pimpl_->config.width, pimpl_->config.height);

  ImGui::CreateContext();
  ImPlot::CreateContext();

  ImGui_Implbgfx_Init(255);
#if BX_PLATFORM_WINDOWS
  ImGui_ImplSDL2_InitForD3D(pimpl_->window);
#elif BX_PLATFORM_OSX
  ImGui_ImplSDL2_InitForMetal(pimpl_->window);
#elif BX_PLATFORM_LINUX
  ImGui_ImplSDL2_InitForVulkan(pimpl_->window);
#endif

  const bgfx::Caps* caps = bgfx::getCaps();

  bool supported = !((caps->supported &
                      (BGFX_CAPS_TEXTURE_2D_ARRAY |
                       BGFX_CAPS_TEXTURE_READ_BACK | BGFX_CAPS_COMPUTE)) == 0U);
  if (!supported) {
    std::cerr << "Not supported machine" << std::endl;
    std::exit(EXIT_FAILURE);
  }

  pimpl_->renderer.Init();
}

Viewer::~Viewer() {
  for (auto* object : pimpl_->objects) {
    object->DeInit();
  }
  pimpl_->renderer.DeInit();

  ImGui_ImplSDL2_Shutdown();
  ImGui_Implbgfx_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  bgfx::shutdown();

  SDL_DestroyWindow(pimpl_->window);
  SDL_Quit();
  std::cout << "[LiVision] Viewer Exit" << std::endl;
}

bool Viewer::SpinOnce() {
  if (!pimpl_->initialized) {
    // FPS計測開始時間の初期化
    pimpl_->last_fps_time = SDL_GetTicks();
    pimpl_->initialized = true;
  }
  for (auto* object : pimpl_->objects) {
    object->UpdateMatrix();
  }

  if (!pimpl_->config.headless) {
    for (auto* object : pimpl_->objects) {
      if (object->IsVisible()) object->OnDraw(pimpl_->renderer);
    }

    // Camera control
    bx::mtxProj(pimpl_->proj, 60.0F,
                static_cast<float>(pimpl_->config.width) /
                    static_cast<float>(pimpl_->config.height),
                0.1F, 1000.0F, bgfx::getCaps()->homogeneousDepth,
                bx::Handedness::Right);

    CameraControl();
    bgfx::setViewTransform(0, pimpl_->view, pimpl_->proj);

    // Event handling
    SDL_Event event = {};
    pimpl_->scroll_delta = 0.0F;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        pimpl_->quit = true;
      }
      if (event.type == SDL_MOUSEWHEEL) {
        pimpl_->scroll_delta = event.wheel.y;
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
        pimpl_->quit = true;
      }
    }
  }

  bgfx::frame();

  // Increment frame count for FPS calculation
  pimpl_->frame_count++;

  // 1秒ごとにFPSを表示
  uint32_t current_time = SDL_GetTicks();
  if (current_time - pimpl_->last_fps_time >= 1000) {
    PrintFPS();
    pimpl_->frame_count = 0;
    pimpl_->last_fps_time = current_time;
  }

  return !pimpl_->quit;
}

void Viewer::PrintFPS() {
  float elapsed_seconds = (SDL_GetTicks() - pimpl_->last_fps_time) / 1000.0F;

  if (pimpl_->config.fps && elapsed_seconds > 0) {
    float current_fps = pimpl_->frame_count / elapsed_seconds;
    std::cout << "[LiVision] FPS: " << current_fps << std::endl;
  }
}

void Viewer::CameraControl() {
  // ImGuiがマウスを使用している場合はカメラ操作を無効化
  if (ImGui::GetIO().WantCaptureMouse) {
    SDL_GetMouseState(&pimpl_->prev_mouse_x, &pimpl_->prev_mouse_y);
    return;
  }

  int mouse_x;
  int mouse_y;
  const uint32_t buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

  int delta_x = mouse_x - pimpl_->prev_mouse_x;
  int delta_y = mouse_y - pimpl_->prev_mouse_y;

  // 回転
  if ((buttons & SDL_BUTTON_LMASK) != 0) {
    pimpl_->cam_yaw += delta_x * kRotScale;
    pimpl_->cam_pitch -= delta_y * kRotScale;
    pimpl_->cam_pitch =
        bx::clamp(pimpl_->cam_pitch, -M_PI_2 + 0.01F, M_PI_2 - 0.01F);
  }

  // パン
  if ((buttons & SDL_BUTTON_MMASK) != 0 || (buttons & SDL_BUTTON_RMASK) != 0) {
    float inv_view[16];
    bx::mtxInverse(inv_view, pimpl_->view);

    bx::Vec3 right = {inv_view[0], inv_view[1], inv_view[2]};
    bx::Vec3 up = {inv_view[4], inv_view[5], inv_view[6]};

    pimpl_->target = bx::mad(right, -delta_x * kPanScale, pimpl_->target);
    pimpl_->target = bx::mad(up, delta_y * kPanScale, pimpl_->target);
  }

  // スクロールで視線方向に target_ を移動
  if (pimpl_->scroll_delta != 0.0F) {
    // 現在のカメラ方向を計算
    bx::Vec3 forward = {bx::cos(pimpl_->cam_pitch) * bx::cos(pimpl_->cam_yaw),
                        bx::cos(pimpl_->cam_pitch) * bx::sin(pimpl_->cam_yaw),
                        bx::sin(pimpl_->cam_pitch)};

    // 視線方向に target_ を移動
    pimpl_->target =
        bx::mad(forward, pimpl_->scroll_delta * kScrollScale, pimpl_->target);
  }

  pimpl_->prev_mouse_x = mouse_x;
  pimpl_->prev_mouse_y = mouse_y;

  bx::Vec3 eye = {
      pimpl_->target.x - (kFixedDistance * bx::cos(pimpl_->cam_pitch) *
                          bx::cos(pimpl_->cam_yaw)),
      pimpl_->target.y - (kFixedDistance * bx::cos(pimpl_->cam_pitch) *
                          bx::sin(pimpl_->cam_yaw)),
      pimpl_->target.z - (kFixedDistance * bx::sin(pimpl_->cam_pitch))};

  const bx::Vec3 up_vec = {0.0F, 0.0F, 1.0F};
  bx::mtxLookAt(pimpl_->view, eye, pimpl_->target, up_vec);
}

void Viewer::AddObject(ObjectBase* object) {
  object->Init();
  auto* container = dynamic_cast<Container*>(object);
  if (container) {
    for (auto* obj : container->GetObjects()) {
      obj->Init();
      AddObject(obj);
    }
  } else {
    pimpl_->objects.push_back(object);
  }
}

}  // namespace livision
