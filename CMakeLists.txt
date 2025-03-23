cmake_minimum_required(VERSION 3.24)

option(SUPERBUILD "Perform a superbuild (or not)" OFF)

project(simulator LANGUAGES CXX)

if (SUPERBUILD)
  set(THIRD_PARTY_BUILD_DIR_NAME build)
  include(third-party/CMakeLists.txt)
  include(cmake/superbuild.cmake)
  return()
endif ()

find_package(glm CONFIG REQUIRED)
find_package(SDL3 REQUIRED CONFIG CMAKE_FIND_ROOT_PATH_BOTH)
find_package(bgfx REQUIRED CONFIG CMAKE_FIND_ROOT_PATH_BOTH)
find_package(imgui.cmake REQUIRED CONFIG CMAKE_FIND_ROOT_PATH_BOTH)

add_executable(${PROJECT_NAME})

file(GLOB SOURCES "src/*.cpp" "src/FastLS/*.cpp" "third-party/sdl-imgui/*.cpp" "third-party/bgfx-imgui/*.cpp")

target_sources(${PROJECT_NAME} PRIVATE ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE include third-party)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(
  ${PROJECT_NAME} PRIVATE glm::glm SDL3::SDL3 bgfx::bgfx bgfx::bx
                          imgui.cmake::imgui.cmake)
target_link_options(
  ${PROJECT_NAME} PRIVATE $<$<BOOL:${EMSCRIPTEN}>:-sMAX_WEBGL_VERSION=2
  -sALLOW_MEMORY_GROWTH=1 --preload-file=shader/embuild/v_simple.bin
  --preload-file=shader/embuild/f_simple.bin>)
target_compile_definitions(
  ${PROJECT_NAME} PRIVATE $<$<BOOL:${EMSCRIPTEN}>:USE_SDL=3>)

add_custom_command(
  TARGET ${PROJECT_NAME}
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shader/build
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shader/build
  VERBATIM)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)