cmake_minimum_required(VERSION 3.16)
project(TenEngine-006-input LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

tenengine_resolve_my_dependencies("006-input" OUT_DEPS MY_DEPS)

add_library(te_input STATIC
  src/ActionId.cpp
  src/AxisId.cpp
  src/BindingTable.cpp
  src/MapDevice.cpp
  src/LoadConfig.cpp
  src/Abstraction.cpp
  src/KeyboardMouse.cpp
  src/Gamepad.cpp
  src/Touch.cpp
)
target_include_directories(te_input PUBLIC include)
target_link_libraries(te_input PRIVATE ${MY_DEPS})

tenengine_add_module_test(
  NAME te_input_test
  MODULE_TARGET te_input
  SOURCES tests/unit/te_input_test.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME test_map_device_load_config
  MODULE_TARGET te_input
  SOURCES tests/unit/test_map_device_load_config.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME test_keyboard_mouse
  MODULE_TARGET te_input
  SOURCES tests/unit/test_keyboard_mouse.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME test_gamepad
  MODULE_TARGET te_input
  SOURCES tests/unit/test_gamepad.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME test_touch
  MODULE_TARGET te_input
  SOURCES tests/unit/test_touch.cpp
  ENABLE_CTEST
)
