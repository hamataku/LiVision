cmake_minimum_required(VERSION 3.21)
project(livision LANGUAGES CXX)

include(GNUInstallDirs)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(LIVISION_BUILD_SHARED "Build as shared library" OFF)
option(LIVISION_BUILD_EXAMPLE "Build example" ON)
option(LIVISION_COMPILE_SHADERS "Compile shaders" ON)
set(LIVISION_SHADER_INSTALL_SUBDIR "livision/shaders" CACHE STRING "Install subdir for shader binaries under CMAKE_INSTALL_DATADIR")

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
find_package(Eigen3 REQUIRED)

# Library target
if(LIVISION_BUILD_SHARED)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_library(${PROJECT_NAME} STATIC)
endif()

# Source files
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/object/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/marker/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot/implot.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot/implot_items.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui.cmake/imgui/backends/imgui_impl_sdl2.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# ライブラリ内部で使うヘッダ（bgfxなど）
set(INTERNAL_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/include_internal
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx.cmake
)

# ユーザーに公開するヘッダ（imgui と implotも含む）
set(PUBLIC_INCLUDE
    ${EIGEN3_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/imgui.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/implot
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${INTERNAL_INCLUDE}   # ライブラリ内部だけ
    PUBLIC  ${PUBLIC_INCLUDE}     # ユーザーに公開
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        LIVISION_SHADER_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shader/bin"
        LIVISION_SHADER_INSTALL_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${LIVISION_SHADER_INSTALL_SUBDIR}"
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        bx
        bgfx
    PUBLIC
        SDL2::SDL2
        imgui.cmake
        Eigen3::Eigen
)

# Example program
if(LIVISION_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# Shader compilation
if(LIVISION_COMPILE_SHADERS)
    set(SHADERC_EXECUTABLE ${CMAKE_BINARY_DIR}/third-party/bgfx.cmake/cmake/bgfx/shaderc)

    if(WIN32)
        set(SHADER_PLATFORM_SUFFIX "win")
    elseif(APPLE)
        set(SHADER_PLATFORM_SUFFIX "mac")
    else()
        set(SHADER_PLATFORM_SUFFIX "linux")
    endif()

    set(SHADER_BINARIES
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/v_simple_${SHADER_PLATFORM_SUFFIX}.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/f_simple_${SHADER_PLATFORM_SUFFIX}.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/v_points_${SHADER_PLATFORM_SUFFIX}.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/f_points_${SHADER_PLATFORM_SUFFIX}.bin
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/compute_ray_cast_${SHADER_PLATFORM_SUFFIX}.bin
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

    install(FILES ${SHADER_BINARIES}
        DESTINATION ${CMAKE_INSTALL_DATADIR}/${LIVISION_SHADER_INSTALL_SUBDIR}
    )
else()
    file(GLOB SHADER_BINARIES_PREBUILT
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/bin/*.bin
    )
    if(SHADER_BINARIES_PREBUILT)
        install(FILES ${SHADER_BINARIES_PREBUILT}
            DESTINATION ${CMAKE_INSTALL_DATADIR}/${LIVISION_SHADER_INSTALL_SUBDIR}
        )
    endif()
endif()

# Alias
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
