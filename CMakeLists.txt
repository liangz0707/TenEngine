# 008-RHI library; C++17, contract-first (specs/_contracts/008-rhi-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_rhi LANGUAGES CXX)

enable_testing()
set(CMAKE_CXX_STANDARD 17)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
tenengine_resolve_my_dependencies("008-rhi" OUT_DEPS MY_DEPS)

add_library(te_rhi STATIC
  src/device/device.cpp
)
target_link_libraries(te_rhi PRIVATE ${MY_DEPS})
target_include_directories(te_rhi PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

tenengine_add_module_test(
  NAME te_rhi_test
  MODULE_TARGET te_rhi
  SOURCES tests/device_create.cpp
  ENABLE_CTEST
)
