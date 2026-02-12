![LiVision](docs/assets/livision_logo.png)

LiVision is a lightweight C++ 3D visualizer for rapid prototyping with ImGui and ImPlot.

[![PPA liblivision-dev](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fapi.launchpad.net%2F1.0%2F~hamatakuzaq%2F%2Barchive%2Fubuntu%2Flivision%3Fws.op%3DgetPublishedBinaries%26status%3DPublished%26binary_name%3Dliblivision-dev%26exact_match%3Dtrue%26order_by_date%3Dtrue&query=entries.0.binary_package_version&label=ppa%3Aliblivision-dev)](https://launchpad.net/~hamatakuzaq/+archive/ubuntu/livision)
[![build](https://github.com/hamataku/LiVision/actions/workflows/build.yml/badge.svg)](https://github.com/hamataku/LiVision/actions/workflows/build.yml)

* 📘 [Documentation (EN)](https://hamataku.github.io/LiVision/)
* 📙 [ドキュメント(日本語)](https://hamataku.github.io/LiVision/ja/)
* 🧩 [API Reference](https://hamataku.github.io/LiVision/api/)

## ✨ Features
- 🧪 **Rapid Prototyping**: Fast setup for visual experiments and ideas.
- 🔷 **Primitive-Rich Drawing**: Plane, box, sphere, cylinder, cone, and more.
- 🗂️ **Container-Based Group Control**: Move/transform multiple objects together.
- 🌐 **3D Object Coverage**: Meshes, models, and **SDF-based** objects.
- 📊 **ImGui + ImPlot UI**: Quick interactive tools and plotting dashboards.

## 🚀 Quick Start
### 📦 Installation
#### Install from PPA
```bash
sudo add-apt-repository ppa:hamatakuzaq/livision
sudo apt update
sudo apt install -y liblivision-dev
```

#### Install from source
```bash
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  pkg-config \
  git \
  libsdl2-dev \
  libeigen3-dev \
  libsdformat14-dev \
  libassimp-dev \
  libcurl4-openssl-dev

git clone https://github.com/hamataku/LiVision.git --recursive
mkdir -p LiVision/build && cd LiVision/build
cmake ..
make -j"$(nproc)"
sudo make install
```

### 🛠️ CMake Example
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

### 💻 Example Program
```cpp
#include "livision/Color.hpp"
#include "livision/Viewer.hpp"
#include "livision/object/Model.hpp"
#include "livision/object/primitives.hpp"

int main() {
  auto viewer = livision::Viewer::Instance({
      .headless = false,  // Set to true for headless mode
      .vsync = true,      // Set to true to enable VSync
      .width = 1280,      // Set the window width
      .height = 720,      // Set the window height
  });

  // UI callback
  viewer->RegisterUICallback([&]() {
    if (ImGui::Button("Close")) {
      viewer->Close();
    }
  });

  auto plane = livision::Plane::Instance(
      livision::ObjectBase::Params{.scale = {20.0, 20.0, 0.0},
                                   .color = livision::color::light_gray});
  viewer->AddObject(plane);

  auto sphere = livision::Sphere::Instance(livision::ObjectBase::Params{
      .pos = {0.0, 0.0, 1.0},
      .scale = {2.0, 2.0, 2.0},
      .color = livision::color::rainbow_z,
      .wire_color = livision::color::black});
  viewer->AddObject(sphere);

  // Model from mesh file.
  auto bunny = livision::Model::InstanceWithFile(
      "path/to/model.stl",
      {.scale = {8.0, 8.0, 8.0}, .color = livision::color::rainbow_z});
  bunny->SetPos(0.0, 0.0, -2.0);
  viewer->AddObject(bunny);

  while (viewer->SpinOnce()) {
  }
  return 0;
}
```

## 📄 License
MIT. See `LICENSE`. Third-party notices are listed in `NOTICE`.

🙏 Special thanks to koide3/iridescene, which greatly inspired this project.
