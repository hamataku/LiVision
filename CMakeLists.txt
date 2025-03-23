cmake_minimum_required(VERSION 3.24)

project(simulator LANGUAGES CXX)

# オプション：外部からインクルードされた場合の挙動制御
option(SIMULATOR_BUILD_STANDALONE "Build as standalone project" ON)
option(SIMULATOR_BUILD_SHARED "Build as shared library" OFF)
option(BGFX_BUILD_EXAMPLES OFF)
option(FASTLS_BUILD_EXAMPLE "Build example if enabled" ON)

# BGFX
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/generated CACHE PATH "Install path prefix, prepended onto install directories.")
add_subdirectory(third-party/bgfx.cmake)
add_subdirectory(third-party/imgui.cmake)

# glm, SDL3
find_package(glm CONFIG REQUIRED)
find_package(SDL3 REQUIRED)

# ライブラリターゲットを作成
if(SIMULATOR_BUILD_SHARED)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()

# Source files
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/sdl-imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui/*.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/third-party"
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# 依存ライブラリをPUBLICとしてリンク（必要なものだけ）
target_link_libraries(
  ${PROJECT_NAME} 
  PUBLIC 
    glm::glm 
    SDL3::SDL3
    bgfx
    bx 
    imgui.cmake
)

# サンプルプログラムのビルド
if(FASTLS_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# エイリアスターゲットの作成（名前空間付き）
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})