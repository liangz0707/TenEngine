# 029-World library; C++17, contract-first (specs/_contracts/029-world-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_world LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

set(WORLD_SOURCES
  src/WorldManager.cpp
  src/WorldModuleInit.cpp
  src/LevelResource.cpp
)

set(WORLD_HEADERS
  include/te/world/WorldTypes.h
  include/te/world/WorldManager.h
  include/te/world/LevelAssetDesc.h
  include/te/world/LevelResource.h
  include/te/world/ModelAssetDesc.h
  include/te/world/ModelResource.h
  include/te/world/ModelComponent.h
  include/te/world/LightComponent.h
  include/te/world/CameraComponent.h
  include/te/world/ReflectionProbeComponent.h
  include/te/world/DecalComponent.h
  include/te/world/WorldModuleInit.h
)

add_library(te_world STATIC ${WORLD_SOURCES} ${WORLD_HEADERS})

target_include_directories(te_world PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

tenengine_resolve_my_dependencies("029-world" OUT_DEPS _world_deps)
target_link_libraries(te_world PUBLIC ${_world_deps})

set_target_properties(te_world PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

source_group("Source Files" FILES ${WORLD_SOURCES})
source_group("Header Files" FILES ${WORLD_HEADERS})
set_target_properties(te_world PROPERTIES PUBLIC_HEADER "${WORLD_HEADERS}")

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
