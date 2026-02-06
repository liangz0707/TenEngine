# 006-Input library; C++17, contract-first (specs/_contracts/006-input-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_input LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CTest at project root so add_test() from helpers are discovered (must be before tests).
enable_testing()

# Use upstream (001-core) cmake helpers for dependency resolution
# Convention: downstream may use sibling upstream's cmake
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" CACHE PATH "TenEngine helpers and dependency map")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Resolve direct dependencies (006-input depends on 001-core and 003-application per TenEngineModuleDependencies.cmake).
# IMPORTANT: Set TENENGINE_SKIP_DEPENDENCY_TESTS before resolving dependencies to prevent upstream test modules from being built.
# This ensures that when upstream modules (001-core, 003-application) are added via add_subdirectory,
# they will check this variable and skip their tests/ subdirectory (per docs/engine-build-module-convention.md §1).
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("006-input" OUT_DEPS MY_DEPS)

# Source files
set(INPUT_SOURCES
  src/Input.cpp
  src/KeyboardState.cpp
  src/MouseState.cpp
  src/GamepadState.cpp
  src/TouchState.cpp
  src/InputBinding.cpp
)

# Header files (for Visual Studio project view)
set(INPUT_HEADERS
  include/te/input/Input.h
  include/te/input/InputTypes.h
)

# Library: te_input (contract: specs/_contracts/006-input-public-api.md)
# Target name must match ABI: te_input (not TenEngine_006_input)
add_library(te_input STATIC
  ${INPUT_SOURCES}
  ${INPUT_HEADERS}
)

target_include_directories(te_input PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Link dependencies (use resolved dependencies from tenengine_resolve_my_dependencies)
# IMPORTANT: te_input's public header (Input.h) includes te/application/Event.h,
# so we need PUBLIC linkage to te_application so that consumers of te_input can access Application headers
# Core also needs to be PUBLIC because:
# 1. Tests may directly include te/core/log.h
# 2. Internal implementation files use te/core/containers.h and te/core/thread.h
# 3. Even though Input.h doesn't expose Core types, consumers may need Core headers for testing/debugging
set(INPUT_PUBLIC_DEPS "")
set(INPUT_PRIVATE_DEPS "")
foreach(dep IN LISTS MY_DEPS)
  # Both te_application and te_core need PUBLIC linkage
  if(dep STREQUAL "te_application" OR dep STREQUAL "te_core")
    list(APPEND INPUT_PUBLIC_DEPS ${dep})
  else()
    list(APPEND INPUT_PRIVATE_DEPS ${dep})
  endif()
endforeach()
target_link_libraries(te_input 
  PUBLIC ${INPUT_PUBLIC_DEPS}
  PRIVATE ${INPUT_PRIVATE_DEPS}
)

# Verify that Application module was correctly resolved
# This check ensures te_application target exists and include paths are available
if(NOT TARGET te_application)
  message(FATAL_ERROR "te_application target not found. Check dependency resolution.")
endif()

# Debug: Print include directories for verification (can be removed later)
get_target_property(APP_INCLUDE_DIRS te_application INTERFACE_INCLUDE_DIRECTORIES)
if(APP_INCLUDE_DIRS)
  message(STATUS "te_application include directories: ${APP_INCLUDE_DIRS}")
else()
  message(WARNING "te_application INTERFACE_INCLUDE_DIRECTORIES not set")
endif()

# Set output directory for Visual Studio
set_target_properties(te_input PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${INPUT_SOURCES})
source_group("Header Files" FILES ${INPUT_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_input PROPERTIES
  PUBLIC_HEADER "${INPUT_HEADERS}"
)

# TODO: Update tests to use new te::input interface
# Old tests use deprecated te_input namespace and are temporarily disabled
# tenengine_add_module_test(
#   NAME te_input_test
#   MODULE_TARGET te_input
#   SOURCES tests/unit/te_input_test.cpp
#   ENABLE_CTEST
# )
# tenengine_add_module_test(
#   NAME test_map_device_load_config
#   MODULE_TARGET te_input
#   SOURCES tests/unit/test_map_device_load_config.cpp
#   ENABLE_CTEST
# )
# tenengine_add_module_test(
#   NAME test_keyboard_mouse
#   MODULE_TARGET te_input
#   SOURCES tests/unit/test_keyboard_mouse.cpp
#   ENABLE_CTEST
# )
# tenengine_add_module_test(
#   NAME test_gamepad
#   MODULE_TARGET te_input
#   SOURCES tests/unit/test_gamepad.cpp
#   ENABLE_CTEST
# )
# tenengine_add_module_test(
#   NAME test_touch
#   MODULE_TARGET te_input
#   SOURCES tests/unit/test_touch.cpp
#   ENABLE_CTEST
# )

# New test using the redesigned interface
tenengine_add_module_test(
  NAME test_input
  MODULE_TARGET te_input
  SOURCES tests/unit/test_input.cpp
  ENABLE_CTEST
)
