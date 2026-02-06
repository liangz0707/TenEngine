# TenEngine-007-subsystems: per docs/build-module-convention.md
# Dependencies: 001-core, 002-object (source by default)

cmake_minimum_required(VERSION 3.16)
project(TenEngine-007-subsystems LANGUAGES CXX VERSION 0.1.0)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# build 测试时不引入上游模块的测试工程（docs/engine-build-module-convention.md §1）
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("007-subsystems" OUT_DEPS MY_DEPS)

# Source files
set(SUBSYSTEMS_SOURCES
  src/registry.cpp
  src/registry_state.cpp
  src/lifecycle.cpp
  src/discovery.cpp
)

# Header files (for Visual Studio project view)
set(SUBSYSTEMS_HEADERS
  include/te/subsystems/descriptor.hpp
  include/te/subsystems/subsystem.hpp
  include/te/subsystems/registry.hpp
  include/te/subsystems/registry.inl
  include/te/subsystems/lifecycle.hpp
  include/te/subsystems/discovery.hpp
  include/te/subsystems/helpers.hpp
  include/te/subsystems/detail/registry_detail.hpp
  include/te/subsystems/detail/registry_state.hpp
)

add_library(te_subsystems STATIC
  ${SUBSYSTEMS_SOURCES}
  ${SUBSYSTEMS_HEADERS}
)

target_include_directories(te_subsystems PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(te_subsystems PRIVATE ${MY_DEPS})

# Set output directory for Visual Studio
set_target_properties(te_subsystems PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${SUBSYSTEMS_SOURCES})
source_group("Header Files" FILES ${SUBSYSTEMS_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_subsystems PROPERTIES
  PUBLIC_HEADER "${SUBSYSTEMS_HEADERS}"
)

# Tests: link only te_subsystems (deps transitive)
tenengine_add_module_test(NAME te_subsystems_registry_test MODULE_TARGET te_subsystems
  SOURCES tests/unit/test_registry.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_subsystems_lifecycle_test MODULE_TARGET te_subsystems
  SOURCES tests/unit/test_lifecycle_order.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_subsystems_discovery_test MODULE_TARGET te_subsystems
  SOURCES tests/unit/test_discovery.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_subsystems_e2e_test MODULE_TARGET te_subsystems
  SOURCES tests/integration/test_subsystem_e2e.cpp ENABLE_CTEST)
