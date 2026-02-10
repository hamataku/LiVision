set(LIVISION_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(sdf main.cpp)

set_target_properties(sdf PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/sdf"
)

target_include_directories(sdf
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(sdf
  PRIVATE LIVISION_EXAMPLE_DIR="${LIVISION_EXAMPLE_DIR}"
)

target_link_libraries(sdf PRIVATE livision)
