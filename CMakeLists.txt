cmake_minimum_required(VERSION 3.24)

project(simulator LANGUAGES CXX)

# BGFX
set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/generated)
option(BGFX_BUILD_EXAMPLES OFF)
add_subdirectory(third-party/bgfx.cmake)
add_subdirectory(third-party/imgui.cmake)

# glm, SDL3
find_package(glm CONFIG REQUIRED)
find_package(SDL3 REQUIRED)

# FASTLS_BUILD_EXAMPLE
option(FASTLS_BUILD_EXAMPLE "Build example if enabled" ON)
if(FASTLS_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# Source files
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/sdl-imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui/*.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party"
)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE glm::glm SDL3::SDL3 bgfx bx imgui.cmake)