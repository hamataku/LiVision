cmake_minimum_required(VERSION 3.24)

project(fastls LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/generated)
option(BGFX_BUILD_EXAMPLES OFF)
add_subdirectory(third-party/bgfx.cmake)
add_subdirectory(third-party/imgui.cmake)

find_package(glm CONFIG REQUIRED)
find_package(SDL3 REQUIRED)

add_executable(${PROJECT_NAME})

file(GLOB SOURCES "src/*.cpp" "src/FastLS/*.cpp" "third-party/sdl-imgui/*.cpp" "third-party/bgfx-imgui/*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include third-party)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE glm::glm SDL3::SDL3 bgfx bx imgui.cmake)