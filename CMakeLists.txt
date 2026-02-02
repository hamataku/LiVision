cmake_minimum_required(VERSION 3.21)

project(livision LANGUAGES CXX)

# Options
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
option(LIVISION_BUILD_SHARED "Build as shared library" OFF)
option(LIVISION_BUILD_EXAMPLE "Build example if enabled" ON)
option(LIVISION_COMPILE_SHADERS "" ON)

add_subdirectory(third-party/SDL)
add_subdirectory(third-party/imgui.cmake)

# bgfx
set(BGFX_AMALGAMATED      OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_EXAMPLES   OFF CACHE INTERNAL "" FORCE)
set(BGFX_CONFIG_DEBUG     OFF CACHE INTERNAL "" FORCE)
set(BGFX_CUSTOM_TARGETS   OFF CACHE INTERNAL "" FORCE)
set(BGFX_INSTALL          OFF CACHE INTERNAL "" FORCE)
set(BGFX_INSTALL_EXAMPLES OFF CACHE INTERNAL "" FORCE)
set(BGFX_USE_DEBUG_SUFFIX OFF CACHE INTERNAL "" FORCE)
set(BGFX_USE_OVR          OFF CACHE INTERNAL "" FORCE)
set(BUILD_SHARED_LIBS     OFF CACHE INTERNAL "" FORCE)
set(BX_AMALGAMATED        OFF CACHE INTERNAL "" FORCE)
add_compile_definitions(BGFX_CONFIG_RENDERER_WEBGPU=0)
if (LIVISION_COMPILE_SHADERS)
  set(BGFX_BUILD_TOOLS ON CACHE INTERNAL "" FORCE)
else()
  set(BGFX_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx.cmake)

# glm
find_package(glm REQUIRED)

# Make library target
if(LIVISION_BUILD_SHARED)
  add_library(${PROJECT_NAME} SHARED)
else()
  add_library(${PROJECT_NAME} STATIC)
endif()

# Source files
file(GLOB SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sim/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot/implot.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot/implot_items.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui.cmake/imgui/backends/imgui_impl_sdl3.cpp"
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
    bx
    bgfx
    glm::glm 
    SDL3::SDL3
    imgui.cmake
)

# sample program
if(LIVISION_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if (LIVISION_COMPILE_SHADERS)
  add_custom_target(
    compile-shaders
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile_shaders.sh
  )
endif()

# Alias target
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
