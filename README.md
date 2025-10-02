# hamazaki_FastLS
Fast Lidar Simulation for multiple robots

# Dependencies
- glm
    - `sudo apt-get install libglm-dev`
- bgfx.cmake
    - Execute the following commands in an appropriate directory
```
git clone https://github.com/bkaradzic/bgfx.cmake.git --recursive
cd bgfx.cmake
mkdir build
cd build
cmake .. -GNinja -DBGFX_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release
ninja
sudo ninja install
```
- SDL3
    - included
- imgui.cmake
    - included

# Build Example Code
```
git clone https://github.com/fkd-lab/hamazaki_FastLS.git --recursive
cd hamazaki_FastLS
./build.sh
./compile-shader.sh
./build/example/fastls_example
```