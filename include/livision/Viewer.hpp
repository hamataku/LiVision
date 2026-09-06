#pragma once

#include <functional>
#include <memory>
#include <string>

#include "livision/Camera.hpp"
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
  bool capture_ui = true;  // Show the built-in Capture section in the panel
};

/**
 * @brief Main rendering window and event loop controller.
 */
class Viewer {
 public:
  using Ptr = std::shared_ptr<Viewer>;
  /**
   * @brief Construct a viewer with the given configuration.
   */
  explicit Viewer(const ViewerConfig& config);
  /**
   * @brief Clean up resources.
   */
  ~Viewer();

  static std::unique_ptr<Viewer> Instance(const ViewerConfig& config) {
    return std::make_unique<Viewer>(config);
  }

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
   * @brief Request a PNG screenshot of the next rendered frame.
   *
   * The image is written while the following SpinOnce() renders, so call
   * SpinOnce() once more before reading the file.
   *
   * @param path Output path. When empty, a timestamped name is generated in
   *   the current directory.
   * @param include_ui Keep the ImGui overlay in the image. Pass false for a
   *   clean render; the overlay is hidden for that one frame only.
   * @return Path the image will be written to.
   */
  std::string SaveScreenshot(const std::string& path = "",
                             bool include_ui = true);
  /**
   * @brief Start recording rendered frames to a video file.
   *
   * Frames are streamed to ffmpeg, which must be installed and on PATH. The
   * container is chosen from the extension; ".gif" is encoded with a
   * generated palette, anything else with H.264. Output is paced on the wall
   * clock, so the video plays at real speed even if rendering is slower.
   *
   * @param path Output path. When empty, a timestamped .mp4 is generated in
   *   the current directory.
   * @param fps Frame rate of the written video.
   * @return Path being recorded to, or an empty string on failure.
   */
  std::string StartRecording(const std::string& path = "", int fps = 30);
  /**
   * @brief Stop the current recording and finish writing the file.
   */
  void StopRecording();
  /**
   * @brief Whether a recording is currently running.
   */
  bool IsRecording() const;
  /**
   * @brief Add an object to be rendered.
   */
  void AddObject(std::shared_ptr<ObjectBase> object);
  /**
   * @brief Register a UI callback (ImGui).
   */
  void RegisterUICallback(std::function<void()> ui_callback);
  /**
   * @brief Set camera controller implementation.
   */
  void SetCameraController(std::unique_ptr<CameraBase> camera);

 private:
  void PrintFPS();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision
