#include "livision/Viewer.hpp"

#include <SDL.h>
#include <SDL_events.h>
#include <SDL_mouse.h>
#include <SDL_syswm.h>
#include <bgfx/bgfx.h>
#include <bgfx/defines.h>
#include <bgfx/platform.h>
#include <bx/math.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <stdexcept>
#include <string>
#include <utility>

#include "imgui_impl_bgfx.h"
#include "livision/Camera.hpp"
#include "livision/Log.hpp"
#include "livision/Renderer.hpp"
#include "livision/internal/capture.hpp"
#include "livision/internal/mesh_buffer_manager.hpp"
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
  return static_cast<uint8_t>((x * 255.0F) + 0.5F);
}

std::string TimestampedName(const char* extension) {
  const auto now = std::chrono::system_clock::now();
  const std::time_t t = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
  localtime_r(&t, &tm);
  char buf[64];
  std::strftime(buf, sizeof(buf), "livision_%Y%m%d_%H%M%S", &tm);
  return std::string(buf) + extension;
}

uint32_t ToRGBA8(const Color& color) {
  const uint8_t r = ToU8(color.base[0]);
  const uint8_t g = ToU8(color.base[1]);
  const uint8_t b = ToU8(color.base[2]);
  return (static_cast<uint32_t>(r) << 24) | (static_cast<uint32_t>(g) << 16) |
         (static_cast<uint32_t>(b) << 8) | 0xFFU;
}
}  // namespace

struct Viewer::Impl {
  SDL_Window* window = nullptr;
  std::vector<std::shared_ptr<ObjectBase>> draw_objects;
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

  internal::CaptureCallback capture;
  char capture_path_buf[512] = {};
  bool pending_screenshot = false;
  std::string clean_screenshot_path;
  bool start_recording_requested = false;
  bool stop_recording_requested = false;

  uint32_t ResetFlags() const {
    uint32_t flags = config.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE;
    if (capture.IsRecording()) {
      flags |= BGFX_RESET_CAPTURE;
    }
    return flags;
  }

  /**
   * @brief Re-apply the reset flags, e.g. after capture is toggled.
   */
  void ApplyReset() {
    bgfx::reset(static_cast<uint32_t>(config.width),
                static_cast<uint32_t>(config.height), ResetFlags());
  }

  /**
   * @brief Built-in Capture section of the control panel.
   */
  void DrawCaptureUI() {
    if (!config.capture_ui) {
      return;
    }
    ImGui::Separator();
    if (!ImGui::CollapsingHeader("Capture")) {
      return;
    }
    ImGui::InputTextWithHint("##capture_path", "output path (blank = auto)",
                             capture_path_buf, sizeof(capture_path_buf));
    if (ImGui::Button("Screenshot")) {
      pending_screenshot = true;
    }
    ImGui::SameLine();
    if (capture.IsRecording()) {
      if (ImGui::Button("Stop recording")) {
        stop_recording_requested = true;
      }
      ImGui::SameLine();
      ImGui::Text("REC %u", capture.RecordedFrames());
    } else if (ImGui::Button("Record")) {
      start_recording_requested = true;
    }
  }

  void Resize(int width, int height) {
    if (width <= 0 || height <= 0) {
      return;
    }

    if (capture.IsRecording()) {
      LogMessage(LogLevel::Warn,
                 "Window resized while recording; stopping the recording so "
                 "the file written so far is kept.");
      capture.EndRecording();
    }

    config.width = width;
    config.height = height;

    bgfx::reset(static_cast<uint32_t>(config.width),
                static_cast<uint32_t>(config.height), ResetFlags());
    bgfx::setViewRect(0, 0, 0, static_cast<uint16_t>(config.width),
                      static_cast<uint16_t>(config.height));
  }
};

Viewer::Viewer(const ViewerConfig& config) : pimpl_(std::make_unique<Impl>()) {
  pimpl_->config = config;
  SetLogLevel(pimpl_->config.log_level);

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error(
        std::string("SDL could not initialize. SDL_Error: ") + SDL_GetError());
  }

  if (pimpl_->config.headless) {
    pimpl_->window = SDL_CreateWindow(
        "Main view", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        pimpl_->config.width, pimpl_->config.height, SDL_WINDOW_HIDDEN);
  } else {
    constexpr uint32_t window_flags = SDL_WINDOW_RESIZABLE;
    pimpl_->window = SDL_CreateWindow(
        "Main view", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        pimpl_->config.width, pimpl_->config.height, window_flags);
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
  bgfx_init.callback = &pimpl_->capture;
  bgfx::init(bgfx_init);
  internal::MeshBufferManager::SetBgfxAlive(true);

  bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
                     ToRGBA8(pimpl_->config.background), 1.0F, 0);
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
  if (pimpl_->capture.IsRecording()) {
    StopRecording();
    bgfx::frame();  // Let bgfx deliver captureEnd() and close the encoder.
  }
  for (auto& object : pimpl_->draw_objects) {
    if (object) {
      object->DeInit();
    }
  }
  pimpl_->draw_objects.clear();
  internal::MeshBufferManager::DestroyAllBuffers();
  pimpl_->renderer.DeInit();

  ImGui_ImplSDL2_Shutdown();
  ImGui_Implbgfx_Shutdown();

  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  internal::MeshBufferManager::SetBgfxAlive(false);
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
  for (const auto& object : pimpl_->draw_objects) {
    object->UpdateMatrix(Eigen::Affine3d::Identity());
  }

  // Event handling. A headless viewer still drains the queue to catch Ctrl+C,
  // but window resizes and camera input only apply to a visible window.
  SDL_Event event = {};
  while (SDL_PollEvent(&event)) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_QUIT) {
      pimpl_->quit = true;
    }
    if (pimpl_->config.headless) {
      continue;
    }
    if (event.type == SDL_WINDOWEVENT &&
        event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
      pimpl_->Resize(event.window.data1, event.window.data2);
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

  // The scene is drawn in headless mode too, so screenshots and recordings
  // work without a visible window. Only the ImGui overlay is skipped.
  for (const auto& object : pimpl_->draw_objects) {
    if (object->IsVisible()) object->OnDraw(pimpl_->renderer);
  }

  if (!pimpl_->config.headless) {
    ImGui_Implbgfx_NewFrame();
    ImGui_ImplSDL2_NewFrame();

    ImGui::NewFrame();
    ImGui::Begin("Control panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    pimpl_->ui_callback();
    pimpl_->DrawCaptureUI();

    ImGui::End();
    ImGui::Render();
    // The overlay is built either way so the user callback still runs; only
    // its submission is skipped for a clean screenshot.
    if (pimpl_->clean_screenshot_path.empty()) {
      ImGui_Implbgfx_RenderDrawLists(ImGui::GetDrawData());
    }
  }

  if (!pimpl_->clean_screenshot_path.empty()) {
    bgfx::requestScreenShot(BGFX_INVALID_HANDLE,
                            pimpl_->clean_screenshot_path.c_str());
    pimpl_->clean_screenshot_path.clear();
  }

  // UI buttons only set flags; the work happens here so it is applied to the
  // frame that is about to be submitted.
  if (pimpl_->pending_screenshot) {
    pimpl_->pending_screenshot = false;
    SaveScreenshot(pimpl_->capture_path_buf);
  }
  if (pimpl_->start_recording_requested) {
    pimpl_->start_recording_requested = false;
    StartRecording(pimpl_->capture_path_buf);
  }
  if (pimpl_->stop_recording_requested) {
    pimpl_->stop_recording_requested = false;
    StopRecording();
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

std::string Viewer::SaveScreenshot(const std::string& path, bool include_ui) {
  const std::string out = path.empty() ? TimestampedName(".png") : path;
  if (include_ui || pimpl_->config.headless) {
    bgfx::requestScreenShot(BGFX_INVALID_HANDLE, out.c_str());
  } else {
    // Deferred: the request is issued once a frame has been rendered without
    // submitting the ImGui draw lists.
    pimpl_->clean_screenshot_path = out;
  }
  return out;
}

std::string Viewer::StartRecording(const std::string& path, int fps) {
  if (pimpl_->capture.IsRecording()) {
    LogMessage(LogLevel::Warn, "Already recording to ",
               pimpl_->capture.RecordingPath());
    return pimpl_->capture.RecordingPath();
  }
  const std::string out = path.empty() ? TimestampedName(".mp4") : path;
  pimpl_->capture.BeginRecording(out, fps);
  // Capture only starts once the swap chain is reset with the capture flag.
  pimpl_->ApplyReset();
  return out;
}

void Viewer::StopRecording() {
  if (!pimpl_->capture.IsRecording()) {
    return;
  }
  pimpl_->capture.EndRecording();
  pimpl_->ApplyReset();
}

bool Viewer::IsRecording() const { return pimpl_->capture.IsRecording(); }

void Viewer::AddObject(std::shared_ptr<ObjectBase> object) {
  if (!object) {
    return;
  }
  object->Init();
  pimpl_->draw_objects.push_back(object);
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
