cmake_minimum_required(VERSION 3.21)
project(livision LANGUAGES CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIVISION_BUILD_SHARED "Build as shared library" OFF)
option(LIVISION_BUILD_EXAMPLE "Build example" ON)
option(LIVISION_COMPILE_SHADERS "Compile shaders" ON)

# Third-party libraries
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
add_compile_definitions(BX_CONFIG_DEBUG=0)
if(LIVISION_COMPILE_SHADERS)
    set(BGFX_BUILD_TOOLS ON CACHE INTERNAL "" FORCE)
else()
    set(BGFX_BUILD_TOOLS OFF CACHE INTERNAL "" FORCE)
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx.cmake)

# System packages
find_package(SDL2 REQUIRED)
find_package(glm REQUIRED)

# Library target
if(LIVISION_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_library(${PROJECT_NAME} STATIC)
endif()

# Source files
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/sim/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot/implot.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot/implot_items.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui.cmake/imgui/backends/imgui_impl_sdl2.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/third-party
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        bx
        bgfx
        glm::glm
    PUBLIC
        SDL2::SDL2
        imgui.cmake
)

# Example program
if(LIVISION_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# Shader compilation
if(LIVISION_COMPILE_SHADERS)
    set(SHADERC_EXECUTABLE ${CMAKE_BINARY_DIR}/third-party/bgfx.cmake/cmake/bgfx/shaderc)

    set(SHADER_BINARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/v_simple_linux.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/f_simple_linux.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/v_points_linux.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/f_points_linux.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/compute_ray_cast_linux.bin
    )

    file(GLOB SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/*.sc
    )

    add_custom_command(
        OUTPUT ${SHADER_BINARIES}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/compile_shaders.sh ${SHADERC_EXECUTABLE}
        DEPENDS ${SHADER_SOURCES} ${SHADERC_EXECUTABLE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Compiling shaders..."
    )

    add_custom_target(compile-shaders ALL DEPENDS ${SHADER_BINARIES})
    add_dependencies(${PROJECT_NAME} compile-shaders)
endif()

# Alias
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
