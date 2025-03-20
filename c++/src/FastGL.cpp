#include "FastGL/FastGL.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_mouse.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <iostream>

#include "FastGL/RayCast.hpp"
#include "FastGL/file_ops.hpp"
#include "FastGL/object/Utils.hpp"
#include "bgfx-imgui/imgui_impl_bgfx.h"
#include "imgui.h"
#include "sdl-imgui/imgui_impl_sdl3.h"

namespace fastgl {

bool FastGL::Run(bool headless) {
  headless_ = headless;
  if (!Init()) {
    return false;
  }

  MainLoop();

  // Cleanup
  utils::DeInit();
  bgfx::destroy(program_);

  ImGui_ImplSDL3_Shutdown();
  ImGui_Implbgfx_Shutdown();

  ImGui::DestroyContext();
  bgfx::shutdown();

  SDL_DestroyWindow(window_);
  SDL_Quit();

  return true;
}

bool FastGL::Init() {
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
    pd.nwh = (void*)(uintptr_t)SDL_GetNumberProperty(
        SDL_GetWindowProperties(window_), SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);
  }
#endif

  bgfx::Init bgfx_init;
  bgfx_init.type = bgfx::RendererType::Count;  // auto choose renderer
  bgfx_init.resolution.width = width_;
  bgfx_init.resolution.height = height_;
  bgfx_init.resolution.reset = BGFX_RESET_NONE;
  bgfx_init.platformData = pd;
  bgfx::init(bgfx_init);

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x6495EDFF, 1.0F,
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

  bool supported = !!(bgfx::getCaps()->supported &
                      (BGFX_CAPS_TEXTURE_2D_ARRAY |
                       BGFX_CAPS_TEXTURE_READ_BACK | BGFX_CAPS_COMPUTE));
  if (!supported) {
    printf("Not supported machine\n");
    return false;
  }

  const std::string shader_root = "shader/build/";

  std::string vshader;
  if (!fileops::ReadFile(shader_root + "v_simple.bin", vshader)) {
    printf("Could not find vertex shader");
    return false;
  }

  std::string fshader;
  if (!fileops::ReadFile(shader_root + "f_simple.bin", fshader)) {
    printf("Could not find fragment shader");
    return false;
  }

  bgfx::ShaderHandle vsh = CreateShader(vshader, "vshader");
  bgfx::ShaderHandle fsh = CreateShader(fshader, "fshader");
  program_ = bgfx::createProgram(vsh, fsh, true);

  PrintBackend();
  utils::Init();

  return true;
}

void FastGL::MouseOperation() {
  if (!ImGui::GetIO().WantCaptureMouse) {
    float mouse_x;
    float mouse_y;
    const uint32_t buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

    // マウスの移動量を計算
    float delta_x = mouse_x - prev_mouse_x_;
    float delta_y = mouse_y - prev_mouse_y_;

    // 左ボタンドラッグで回転
    if ((buttons & SDL_BUTTON_LMASK) != 0) {
      cam_yaw_ += (-delta_x) * rot_scale_;
      cam_pitch_ += (-delta_y) * rot_scale_;
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
              100.0F, bgfx::getCaps()->homogeneousDepth);

  bgfx::setViewTransform(0, view, proj);
}

void FastGL::MainLoop() {
  bool quit = false;

  if (scene_set_) {
    scene_->Init();
    scene_set_ = false;
  }
  scene_->AddMeshList();
  ray_cast.Init();
  while (!quit) {
    if (scene_) {
      scene_->Update();
      if (!headless_) {
        scene_->Draw(program_);
      }
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) {
        quit = true;
        break;
      }
      if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        zoom_distance_ += event.wheel.y * zoom_scale_;
      }
    }

    ImGui_Implbgfx_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    // ImGui::NewFrame();
    // ImGui::ShowDemoWindow();  // your drawing here
    // ImGui::Render();
    // ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());

    MouseOperation();

    bgfx::frame();

    PrintFPS();
  }
}

void FastGL::PrintFPS() {
  static int fps_print_counter = 0;
  if (++fps_print_counter % 1000 == 0) {
    Uint64 current_counter = SDL_GetPerformanceCounter();
    float delta_time = static_cast<float>(current_counter - last_counter_) /
                       SDL_GetPerformanceFrequency();
    last_counter_ = current_counter;
    float fps = 1.0F / delta_time * 1000.0F;
    printf("Current FPS: %.1f\n", fps);
    fps_print_counter = 0;
  }
}

bgfx::ShaderHandle FastGL::CreateShader(const std::string& shader,
                                        const char* name) {
  const bgfx::Memory* mem = bgfx::copy(shader.data(), shader.size());
  const bgfx::ShaderHandle handle = bgfx::createShader(mem);
  bgfx::setName(handle, name);
  return handle;
}

void FastGL::PrintBackend() {
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

}  // namespace fastgl
