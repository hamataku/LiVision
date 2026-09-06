# Python bindings (pybind11). Enabled with -DLIVISION_BUILD_PYTHON=ON, or
# automatically when built through pip / scikit-build-core (SKBUILD is set).

set(PYBIND11_FINDPYTHON ON)
find_package(Python 3.8 COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 2.9 CONFIG REQUIRED)

pybind11_add_module(_livision MODULE src/bindings.cpp)
target_link_libraries(_livision PRIVATE livision::livision)
target_compile_definitions(_livision PRIVATE LIVISION_VERSION="${PROJECT_VERSION}")

if(SKBUILD)
    # Wheel layout: livision/_livision.so, livision/liblivision.so, livision/shaders/
    set_target_properties(_livision PROPERTIES INSTALL_RPATH "$ORIGIN")
    install(TARGETS _livision LIBRARY DESTINATION .)
else()
    # In-tree layout usable with PYTHONPATH=<build>/python
    set(LIVISION_PY_OUT ${CMAKE_BINARY_DIR}/python/livision)
    set_target_properties(_livision PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${LIVISION_PY_OUT}
    )
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/livision/__init__.py
                   ${LIVISION_PY_OUT}/__init__.py COPYONLY)
endif()
