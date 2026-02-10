set(LIVISION_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(basic main.cpp)

set_target_properties(basic PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/basic"
)

target_include_directories(basic
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(basic
  PRIVATE LIVISION_EXAMPLE_DIR="${LIVISION_EXAMPLE_DIR}"
)

target_link_libraries(basic PRIVATE livision)
