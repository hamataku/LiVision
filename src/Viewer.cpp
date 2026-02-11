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
#include <stdexcept>
#include <algorithm>
#include <utility>

#include "imgui_impl_bgfx.h"
#include "livision/Container.hpp"
#include "livision/Camera.hpp"
#include "livision/Log.hpp"
#include "livision/Renderer.hpp"
#include "livision/imgui/imgui_impl_sdl2.h"

namespace livision {

namespace {
uint8_t ToU8(float x) {
  if (x < 0.0F) {
    return 0;
  }
  if (x > 1.0F) {
    return 255;
  }
  return static_cast<uint8_t>(x * 255.0F + 0.5F);
}

uint32_t ToRgba8(const Color& color) {
  const uint8_t r = ToU8(color.base[0]);
  const uint8_t g = ToU8(color.base[1]);
  const uint8_t b = ToU8(color.base[2]);
  return (static_cast<uint32_t>(r) << 24) | (static_cast<uint32_t>(g) << 16) |
         (static_cast<uint32_t>(b) << 8) | 0xFFU;
}

void RegisterObjectRecursive(ObjectBase* object,
                             std::vector<ObjectBase*>& draw_objects) {
  object->Init();
  draw_objects.push_back(object);
  auto* container = dynamic_cast<Container*>(object);
  if (!container) {
    return;
  }
  for (const auto& child : container->GetObjects()) {
    RegisterObjectRecursive(child.get(), draw_objects);
  }
}
}  // namespace

struct Viewer::Impl {
  SDL_Window* window = nullptr;
  std::vector<std::shared_ptr<ObjectBase>> owned_objects;
  std::vector<ObjectBase*> draw_objects;
  ViewerConfig config;
  Renderer renderer;
  std::function<void()> ui_callback = []() {};

  bool initialized = false;
  bool quit = false;

  int frame_count = 0;
  uint32_t last_fps_time = 0;
  uint32_t last_frame_time = 0;

  std::unique_ptr<CameraBase> camera = std::make_unique<MouseOrbitCamera>();
  float view[16] = {};
  float proj[16];
};

Viewer::Viewer(const ViewerConfig& config) : pimpl_(std::make_unique<Impl>()) {
  pimpl_->config = config;
  SetLogLevel(pimpl_->config.log_level);

  if (pimpl_->config.headless) {
    pimpl_->config.width = 1;
    pimpl_->config.height = 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error(
        std::string("SDL could not initialize. SDL_Error: ") + SDL_GetError());
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
    throw std::runtime_error(
        std::string("Window could not be created. SDL_Error: ") +
        SDL_GetError());
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
    switch (wm_info.subsystem) {
      case SDL_SYSWM_X11:
        pd.ndt = wm_info.info.x11.display;  // Display*
        pd.nwh = (void*)static_cast<uintptr_t>(
            wm_info.info.x11.window);  // Window (XID)
        break;
      case SDL_SYSWM_WAYLAND:
        pd.ndt = wm_info.info.wl.display;  // wl_display*
        pd.nwh = wm_info.info.wl.surface;  // wl_surface*
        break;
      default:
        throw std::runtime_error("Unsupported SDL window subsystem on Linux");
    }
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

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                     ToRgba8(pimpl_->config.background), 1.0F, 0);
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
    throw std::runtime_error("Not supported machine");
  }

  pimpl_->renderer.Init();
}

Viewer::~Viewer() {
  for (auto* object : pimpl_->draw_objects) {
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
  LogMessage(LogLevel::Info, "Viewer Exit");
}

bool Viewer::SpinOnce() {
  if (!pimpl_->initialized) {
    // FPS計測開始時間の初期化
    pimpl_->last_fps_time = SDL_GetTicks();
    pimpl_->last_frame_time = pimpl_->last_fps_time;
    pimpl_->initialized = true;
  }
  for (auto* object : pimpl_->draw_objects) {
    object->UpdateMatrix();
  }

  if (!pimpl_->config.headless) {
    // Event handling
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT) {
        pimpl_->quit = true;
      }
      if (pimpl_->camera) {
        pimpl_->camera->HandleEvent(event);
      }
    }

    const uint32_t now = SDL_GetTicks();
    const float delta_time_sec =
        static_cast<float>(now - pimpl_->last_frame_time) / 1000.0F;
    pimpl_->last_frame_time = now;

    // Camera control
    bx::mtxProj(pimpl_->proj, 60.0F,
                static_cast<float>(pimpl_->config.width) /
                    static_cast<float>(pimpl_->config.height),
                0.1F, 1000.0F, bgfx::getCaps()->homogeneousDepth,
                bx::Handedness::Right);

    if (pimpl_->camera) {
      CameraInputContext input_context;
      input_context.want_capture_mouse = ImGui::GetIO().WantCaptureMouse;
      input_context.want_capture_keyboard = ImGui::GetIO().WantCaptureKeyboard;
      input_context.delta_time_sec = delta_time_sec;
      const float* view = pimpl_->camera->Update(input_context);
      std::copy(view, view + 16, pimpl_->view);
    }

    pimpl_->renderer.SetCameraViewMatrix(pimpl_->view);
    bgfx::setViewTransform(0, pimpl_->view, pimpl_->proj);

    for (auto* object : pimpl_->draw_objects) {
      if (object->IsVisible()) object->OnDraw(pimpl_->renderer);
    }

    // Render ImGui
    ImGui_Implbgfx_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Control panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    pimpl_->ui_callback();

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

  float current_fps = pimpl_->frame_count / elapsed_seconds;
  LogMessage(LogLevel::Debug, "FPS: ", current_fps);
}

void Viewer::Close() { pimpl_->quit = true; }

void Viewer::AddObject(std::shared_ptr<ObjectBase> object) {
  if (!object) {
    return;
  }
  pimpl_->owned_objects.push_back(object);
  RegisterObjectRecursive(object.get(), pimpl_->draw_objects);
}

void Viewer::RegisterUICallback(std::function<void()> ui_callback) {
  pimpl_->ui_callback = std::move(ui_callback);
}

void Viewer::SetCameraController(std::unique_ptr<CameraBase> camera) {
  if (camera) {
    pimpl_->camera = std::move(camera);
  }
}

}  // namespace livision
