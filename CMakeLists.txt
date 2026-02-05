# 013-Resource + 001-Core (in-tree); C++17, contract-first (specs/_contracts/013-resource-ABI.md).
# Build root: worktree root. Dependencies: 001-core (in-tree), 002-object, 028-texture (resolve via TENENGINE_*_DIR when present).
cmake_minimum_required(VERSION 3.16)
project(TenEngine-013-resource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# --- 001-Core (in-tree for this worktree) ---
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
target_include_directories(te_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# --- 013-Resource (depends on te_core; 002-object and 028-texture via TenEngineHelpers when TENENGINE_*_DIR set) ---
add_library(te_resource
  src/ResourceManager.cpp
  src/Resource.cpp
  src/ResourceId.cpp
)
target_include_directories(te_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_resource PRIVATE te_core)

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
