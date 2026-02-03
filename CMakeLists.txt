# 001-Core library; C++17, contract-first (specs/_contracts/001-core-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(te_core
  src/alloc.cpp
  src/engine.cpp
  src/thread.cpp
  src/platform.cpp
  src/log.cpp
  src/math.cpp
  src/containers.cpp
  src/module_load.cpp
)
target_include_directories(te_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
