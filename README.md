# hamazaki_FastLS
Fast Lidar Simulation for multiple robots

# dependencies
- glm
    - `sudo apt-get install libglm-dev`
- bgfx.cmake
    - git clone https://github.com/bkaradzic/bgfx.cmake.git
    - cd bgfx.cmake
    - mkdir build
    - cd build
    - cmake .. -GNinja -DBGFX_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
    - ninja
    - sudo ninja install
- SDL3
    - included
- imgui.cmake
    - included

# example code build
- ./build.sh
- ./compile-shader.sh
- ./build/example/fastls_example