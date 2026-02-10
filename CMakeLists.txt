cmake_minimum_required(VERSION 3.21)
project(livision VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  # Only set default when user didn't specify anything
  if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
  endif()
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(LIVISION_BUILD_SHARED ON CACHE BOOL "Build as shared library" FORCE)
option(LIVISION_BUILD_EXAMPLE "Build example" ON)
option(LIVISION_COMPILE_SHADERS "Compile shaders at build time" ON)
option(LIVISION_INSTALL_PRECOMPILED_SHADERS
       "Install precompiled shaders from shader/bin when not compiling"
       ON)
set(LIVISION_SHADER_INSTALL_SUBDIR "livision/shaders" CACHE STRING "Install subdir for shader binaries under CMAKE_INSTALL_DATADIR")

# bgfx
set(BGFX_LIBRARY_TYPE "STATIC" CACHE STRING "" FORCE)
set(BGFX_AMALGAMATED      OFF CACHE INTERNAL "" FORCE)
set(BGFX_BUILD_EXAMPLES   OFF CACHE INTERNAL "" FORCE)
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

set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/object/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/marker/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/imgui/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/implot/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui/*.cpp"
)

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

# ライブラリ内部で使うヘッダ（bgfxなど）
set(INTERNAL_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/internal
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx-imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/third-party/bgfx.cmake
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${INTERNAL_INCLUDE}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    LIVISION_SHADER_INSTALL_DIR="${CMAKE_INSTALL_FULL_DATADIR}/${LIVISION_SHADER_INSTALL_SUBDIR}"
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        $<BUILD_INTERFACE:bx>
        $<BUILD_INTERFACE:bgfx>
    PUBLIC
        SDL2::SDL2
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
    )

    file(GLOB SHADER_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/shader/*.sc
    )

    add_custom_command(
        OUTPUT ${SHADER_BINARIES}
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/scripts/compile_shaders.sh ${SHADERC_EXECUTABLE}
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
    if(LIVISION_INSTALL_PRECOMPILED_SHADERS)
        if(SHADER_BINARIES_PREBUILT)
            install(FILES ${SHADER_BINARIES_PREBUILT}
                DESTINATION ${CMAKE_INSTALL_DATADIR}/${LIVISION_SHADER_INSTALL_SUBDIR}
            )
        else()
            message(WARNING "LIVISION_COMPILE_SHADERS=OFF but no precompiled shaders found in shader/bin")
        endif()
    endif()
endif()

# Alias
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

set(LIVISION_EXPORT_TARGETS ${PROJECT_NAME})

install(TARGETS ${LIVISION_EXPORT_TARGETS}
    EXPORT livisionTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/include/livision/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/livision
)

# --- CMake package configuration for find_package(livision) ---
include(CMakePackageConfigHelpers)

set(CONFIG_INSTALL_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
set(PACKAGE_NAME ${PROJECT_NAME})
set(PACKAGE_TARGETS_FILE ${PACKAGE_NAME}Targets.cmake)

install(EXPORT livisionTargets
    FILE ${PACKAGE_TARGETS_FILE}
    NAMESPACE ${PACKAGE_NAME}::
    DESTINATION ${CONFIG_INSTALL_DIR}
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/livisionConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}ConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}Config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PACKAGE_NAME}ConfigVersion.cmake
    DESTINATION ${CONFIG_INSTALL_DIR}
)
