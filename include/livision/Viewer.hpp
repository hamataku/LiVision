#pragma once
#include <memory>

#include "livision/ObjectBase.hpp"

namespace livision {

struct ViewerConfig {
  bool headless = false;  // Headless mode (no window)
  bool vsync = true;      // Enable VSync
  int width = 1280;       // Window width
  int height = 720;       // Window height
  bool fps = false;       // Display FPS
};

class Viewer {
 public:
  explicit Viewer(const ViewerConfig& config);
  ~Viewer();

  bool SpinOnce();
  void AddObject(ObjectBase* object);

 private:
  void PrintFPS();
  void CameraControl();
  struct Impl;
  std::unique_ptr<Impl> pimpl_;
};

}  // namespace livision