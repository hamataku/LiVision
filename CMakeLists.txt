cmake_minimum_required(VERSION 3.24)

project(fastls LANGUAGES CXX)

# Options
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
option(FASTLS_BUILD_SHARED "Build as shared library" OFF)
option(FASTLS_BUILD_EXAMPLE "Build example if enabled" ON)

add_subdirectory(third-party/SDL)
add_subdirectory(third-party/imgui.cmake)

# bgfx, glm
find_package(bgfx CONFIG REQUIRED)
find_package(glm REQUIRED)

# ライブラリターゲットを作成
if(FASTLS_BUILD_SHARED)
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
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/third-party)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/third-party)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# PUBLIC link
target_link_libraries(
  ${PROJECT_NAME} 
  PUBLIC 
    bgfx::bx
    bgfx::bgfx
    glm::glm 
    SDL3::SDL3
    imgui.cmake
)

# sample program
if(FASTLS_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# エイリアスターゲットの作成（名前空間付き）
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})