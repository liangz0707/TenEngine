# TenEngine-007-subsystems: per docs/build-module-convention.md
# Dependencies: 001-core, 002-object (source by default)

cmake_minimum_required(VERSION 3.16)
project(TenEngine-007-subsystems LANGUAGES CXX VERSION 0.1.0)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

tenengine_resolve_my_dependencies("007-subsystems" OUT_DEPS MY_DEPS)

add_library(te_subsystems STATIC
  src/registry.cpp
  src/registry_state.cpp
  src/lifecycle.cpp
  src/discovery.cpp
)
target_include_directories(te_subsystems PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(te_subsystems PRIVATE ${MY_DEPS})

# Tests: link only te_subsystems (deps transitive)
tenengine_add_module_test(NAME te_subsystems_registry_test MODULE_TARGET te_subsystems
  SOURCES tests/unit/test_registry.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_subsystems_lifecycle_test MODULE_TARGET te_subsystems
  SOURCES tests/unit/test_lifecycle_order.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_subsystems_discovery_test MODULE_TARGET te_subsystems
  SOURCES tests/unit/test_discovery.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_subsystems_e2e_test MODULE_TARGET te_subsystems
  SOURCES tests/integration/test_subsystem_e2e.cpp ENABLE_CTEST)
