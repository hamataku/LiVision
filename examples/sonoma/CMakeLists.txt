set(LIVISION_EXAMPLE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(sonoma main.cpp)

set_target_properties(sonoma PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/sonoma"
)

target_include_directories(sonoma
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_compile_definitions(sonoma
  PRIVATE LIVISION_EXAMPLE_DIR="${LIVISION_EXAMPLE_DIR}"
)

target_link_libraries(sonoma PRIVATE livision)
