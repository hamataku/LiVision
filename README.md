![LiVision](docs/assets/livision_logo.png)

LiVision is a lightweight C++ 3D visualizer for rapid prototyping with imgui & implot.

[![build](https://github.com/hamataku/LiVision/actions/workflows/build.yml/badge.svg)](https://github.com/hamataku/LiVision/actions/workflows/build.yml)

* [Documentation(en)](https://hamataku.github.io/LiVision/)
* [ドキュメント(日本語)](https://hamataku.github.io/LiVision/ja/)
* [API Reference](https://hamataku.github.io/LiVision/api/)

## Features
- Simple object hierarchy with transforms and visibility control
- Mesh rendering with optional wireframe
- STL (binary) loading
- ImGui/ImPlot UI callback
- Shader search path control and precompiled shader support

## Requirements
- SDL2
- Eigen3
- bgfx(submodule)

## Installation
### Install from PPA
```bash
sudo add-apt-repository ppa:hamatakuzaq/livision
sudo apt update
sudo apt install -y liblivision-dev
```

### Install from source
```bash
sudo apt install -y libeigen3-dev libsdl2-dev
git clone https://github.com/hamataku/LiVision.git --recursive
mkdir LiVision/build && cd LiVision/build
cmake ..
make -j
sudo make install
```

## Usage
### CMakeLists.txt
```cmake
# Find package
find_package(livision REQUIRED)

# Add include dirs and link libraries
add_executable(your_program
  src/your_program.cpp
)
target_link_libraries(your_program
  livision::livision
)
```

```cpp
#include "livision/Color.hpp"
#include "livision/Viewer.hpp"
#include "livision/object/Model.hpp"
#include "livision/object/primitives.hpp"

int main() {
  livision::Viewer viewer{{
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
  }};

  // UI callback
  viewer.RegisterUICallback([&viewer]() {
    if (ImGui::Button("Close")) {
      viewer.Close();
    }
  });

  // AddObject accepts shared_ptr only.
  auto plane = livision::Plane::Instance(
      livision::ObjectBase::Params{.scale = {20.0, 20.0, 0.0},
                                   .color = livision::color::light_gray});
  viewer.AddObject(plane);

  auto sphere = livision::Sphere::Instance(livision::ObjectBase::Params{
      .pos = {0.0, 0.0, 1.0},
      .scale = {2.0, 2.0, 2.0},
      .color = livision::color::rainbow_z,
      .wire_color = livision::color::black});
  viewer.AddObject(sphere);

  // Model from mesh file.
  auto bunny = livision::Model::InstanceWithFile(
      "path/to/model.stl",
      {.scale = {8.0, 8.0, 8.0}, .color = livision::color::rainbow_z});
  bunny->SetPos(0.0, 0.0, -2.0);
  viewer.AddObject(bunny);

  while (viewer.SpinOnce()) {
  }
  return 0;
}
```

## CMake Options
- `LIVISION_BUILD_SHARED` (default: `OFF`)
- `LIVISION_BUILD_EXAMPLE` (default: `ON`)
- `LIVISION_COMPILE_SHADERS` (default: `ON`)
- `LIVISION_INSTALL_PRECOMPILED_SHADERS` (default: `ON`)

## License
MIT. See `LICENSE`. Third-party notices are listed in `NOTICE`.
