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

## クイックスタート
### インストール
#### Install from PPA
```bash
sudo add-apt-repository ppa:hamatakuzaq/livision
sudo apt update
sudo apt install -y liblivision-dev
```

#### Install from source
```bash
sudo apt install -y libeigen3-dev libsdl2-dev
git clone https://github.com/hamataku/LiVision.git --recursive
mkdir LiVision/build && cd LiVision/build
cmake ..
make -j
sudo make install
```

### CMakeの例
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

### サンプルプログラム
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

### ライセンス
MIT. See `LICENSE`. Third-party notices are listed in `NOTICE`.

## 基本操作
### ビューワーインスタンスの作成
```cpp
auto viewer = livision::Viewer::Instance({
    .headless = false,
    .vsync = true,
    .width = 1280,
    .height = 720,
});
```

### オブジェクトの登録
`AddObject` は `std::shared_ptr<ObjectBase>` のみを受け取ります。

```cpp
auto sphere = livision::Sphere::Instance(
    {.pos = {0.0, 0.0, 1.0}, .scale = {2.0, 2.0, 2.0}});
viewer->AddObject(sphere);
```

### ImGuiコールバックの登録
```cpp
viewer->RegisterUICallback([&]() {
  if (ImGui::Button("Close")) viewer->Close();
});
```

## 色設定
`livision::Color` と `livision::color::*` を使います。

- 固定色: `livision::color::red`, `livision::color::light_gray` など
- レインボー色: `livision::color::rainbow_x/y/z`
- 透過/非表示: `livision::color::transparent`, `livision::color::invisible`

```cpp
auto box = livision::Box::Instance({
    .color = livision::color::rainbow_z,
    .wire_color = livision::color::black,
});
```

## オブジェクト一覧
### Primitives
- `Plane`
- `Box`
- `Sphere`
- `Cylinder`
- `Cone`

### Objects
- `Mesh`
- `Model`
- `Text`
- `Drone`

### Markers
- `Arrow`
- `Grid`
- `Path`
- `Odometry`
- `DegeneracyIndicator`
- `PointCloud`

## コンテナ
`Container` は子オブジェクトを `shared_ptr` で保持する階層オブジェクトです。

- `Container::AddObject(std::shared_ptr<ObjectBase>)`
- `Container::GetObjects()`
- `Container::ClearObjects()`

`Model` や `Drone`、`Odometry` は `Container` ベースで実装されています。

## カメラ操作クラス
`Viewer` はカメラコントローラ差し替えに対応しています。

- `CameraBase` (抽象基底)
- `MouseOrbitCamera` (デフォルト)
- `KeyboardOrbitCamera`

```cpp
viewer->SetCameraController(std::make_unique<livision::KeyboardOrbitCamera>());
```

## ドキュメントページ
- クイックスタート: `docs/quick_start.md`
- 基本操作: `docs/basic_operations.md`
- 色設定: `docs/colors.md`
- オブジェクト一覧: `docs/objects.md`
- コンテナ: `docs/container.md`
- カメラ操作クラス: `docs/camera.md`

## CMake Options
- `LIVISION_BUILD_SHARED` (default: `OFF`)
- `LIVISION_BUILD_EXAMPLE` (default: `ON`)
- `LIVISION_COMPILE_SHADERS` (default: `ON`)
- `LIVISION_INSTALL_PRECOMPILED_SHADERS` (default: `ON`)
