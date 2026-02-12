set(LIVISION_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(objects main.cpp)

set_target_properties(objects PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/objects"
)

target_include_directories(objects
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(objects
  PRIVATE LIVISION_EXAMPLE_DIR="${LIVISION_EXAMPLE_DIR}"
)

target_link_libraries(objects PRIVATE livision)
