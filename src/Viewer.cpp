#include "livision/Viewer.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
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
#include "livision/internal/Renderer.hpp"

namespace livision {
struct Viewer::Impl {
  SDL_Window* window = nullptr;
  std::vector<ObjectBase*> objects;
  ViewerConfig config;
  internal::Renderer renderer;

  bool initialized = false;
  bool quit = false;
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

  // utils::Init();
}

Viewer::~Viewer() {
  // Cleanup
  pimpl_->renderer.DeInit();
  // utils::DeInit();

  ImGui_ImplSDL2_Shutdown();
  ImGui_Implbgfx_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  bgfx::shutdown();

  SDL_DestroyWindow(pimpl_->window);
  SDL_Quit();
  std::cout << "### Viewer Exit ###" << std::endl;
}

bool Viewer::SpinOnce() {
  if (!pimpl_->initialized) {
    // // FPS計測開始時間の初期化
    // last_fps_time_ = SDL_GetTicks();
    pimpl_->initialized = true;
  }
  for (auto* object : pimpl_->objects) {
    object->UpdateMatrix();
  }

  if (!pimpl_->config.headless) {
    for (auto* object : pimpl_->objects) {
      // if (object->IsVisible()) object->Draw();
    }

    // Camera control
    // Draw(program_);

    // bx::mtxProj(proj_, 60.0F,
    //             static_cast<float>(width_) / static_cast<float>(height_),
    //             0.1F, 1000.0F, bgfx::getCaps()->homogeneousDepth,
    //             bx::Handedness::Right);

    // CameraControl();
    // bgfx::setViewTransform(0, view_, proj_);

    // Event handling
    // SDL_Event event = {};
    // scroll_delta_ = 0.0F;
    // while (SDL_PollEvent(&event)) {
    //   ImGui_ImplSDL2_ProcessEvent(&event);
    //   if (event.type == SDL_QUIT) {
    //     quit_ = true;
    //   }
    //   if (event.type == SDL_MOUSEWHEEL) {
    //     scroll_delta_ = event.wheel.y;
    //   }
    //   // scene_->EventHandler(event);
    // }

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

  // フレームカウントを増やす
  // frame_count_++;

  // // 1秒ごとにFPSを表示
  // uint64_t current_time = SDL_GetTicks();
  // if (current_time - last_fps_time_ >= 1000) {
  //   PrintFPS();
  //   frame_count_ = 0;
  //   last_fps_time_ = current_time;
  // }

  return !pimpl_->quit;
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
