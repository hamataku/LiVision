# Quick Start

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

## CMake Example

```cmake
find_package(livision REQUIRED)

add_executable(your_program src/your_program.cpp)
target_link_libraries(your_program PRIVATE livision::livision)
```

## Sample Program

```cpp
#include "livision/Color.hpp"
#include "livision/Viewer.hpp"
#include "livision/object/primitives.hpp"

int main() {
  auto viewer = livision::Viewer::Instance({
      .headless = false,
      .vsync = true,
      .width = 1280,
      .height = 720,
  });

  auto plane = livision::Plane::Instance(
      {.scale = {20.0, 20.0, 0.0}, .color = livision::color::light_gray});
  viewer->AddObject(plane);

  while (viewer->SpinOnce()) {
  }
  return 0;
}
```

## License

MIT. See `LICENSE` and `NOTICE`.
