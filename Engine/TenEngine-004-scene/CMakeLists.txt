# 004-Scene library; C++17, contract-first (specs/_contracts/004-scene-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_scene LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include CMake helpers for dependency resolution
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Source files
set(SCENE_SOURCES
  src/SceneManager.cpp
  src/SceneWorld.cpp
  src/SpatialQuery.cpp
  src/DynamicNodeManager.cpp
  src/StaticNodeManager.cpp
  src/Octree.cpp
  src/Quadtree.cpp
)

# Header files (for Visual Studio project view)
set(SCENE_HEADERS
  include/te/scene/SceneTypes.h
  include/te/scene/SceneDesc.h
  include/te/scene/ISceneNode.h
  include/te/scene/SceneManager.h
  include/te/scene/SceneWorld.h
  include/te/scene/SpatialQuery.h
  include/te/scene/NodeManager.h
  include/te/scene/DynamicNodeManager.h
  include/te/scene/StaticNodeManager.h
  include/te/scene/SpatialIndex.h
  include/te/scene/Octree.h
  include/te/scene/Quadtree.h
)

add_library(te_scene STATIC
  ${SCENE_SOURCES}
  ${SCENE_HEADERS}
)

target_include_directories(te_scene PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Resolve dependencies: Core module only (no Object dependency - Scene is pure algorithm module)
tenengine_resolve_my_dependencies("004-scene" OUT_DEPS _scene_deps)
target_link_libraries(te_scene PUBLIC ${_scene_deps})

# Set output directory for Visual Studio
set_target_properties(te_scene PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${SCENE_SOURCES})
source_group("Header Files" FILES ${SCENE_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_scene PROPERTIES
  PUBLIC_HEADER "${SCENE_HEADERS}"
)

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
