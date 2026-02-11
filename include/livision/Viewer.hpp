#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include "livision/Color.hpp"
#include "livision/Log.hpp"
#include "livision/ObjectBase.hpp"
#include "livision/imgui/imgui.h"
#include "livision/implot/implot.h"

namespace livision {

/**
 * @brief Viewer configuration options.
 */
struct ViewerConfig {
  bool headless = false;                 // Headless mode (no window)
  bool vsync = true;                     // Enable VSync
  int width = 1280;                      // Window width
  int height = 720;                      // Window height
  Color background = color::light_gray;  // Background color (RGB is used)
  LogLevel log_level = LogLevel::Info;   // Log level
};

/**
 * @brief Main rendering window and event loop controller.
 */
class Viewer {
 public:
  /**
   * @brief Construct a viewer with the given configuration.
   */
  explicit Viewer(const ViewerConfig& config);
  /**
   * @brief Clean up resources.
   */
  ~Viewer();

  /**
   * @brief Run a single frame.
   * @return True while the viewer should continue running.
   */
  bool SpinOnce();
  /**
   * @brief Request viewer shutdown.
   */
  void Close();
  /**
   * @brief Add an object to be rendered.
   */
  void AddObject(ObjectBase* object);
  /**
   * @brief Register a UI callback (ImGui).
   */
  void RegisterUICallback(std::function<void()> ui_callback);

 private:
  void PrintFPS();
  void CameraControl();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision
