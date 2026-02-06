# 013-Resource module; per docs/engine-build-module-convention.md §2.5.
# Depends on te_core and te_object. Public headers (include/te/resource/*.h) only include
# own-module headers (te/resource/*); no #include te/object or te/core in public API.
cmake_minimum_required(VERSION 3.16)
project(te_resource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(TE_RESOURCE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(te_resource
  src/Resource.cpp
  src/ResourceId.cpp
  src/ResourceManager.cpp
)
target_include_directories(te_resource PUBLIC
  $<BUILD_INTERFACE:${TE_RESOURCE_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)
# List public headers so they appear in IDE (e.g. Visual Studio solution explorer).
target_sources(te_resource PUBLIC
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/Resource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/ResourceId.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/ResourceTypes.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/ResourceManager.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/ResourceLoader.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/ResourceSerializer.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/ResourceImporter.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/FResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/RResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/DResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/TextureResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/MeshResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/MaterialResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/EffectResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/TerrainResource.h
  ${TE_RESOURCE_INCLUDE_DIR}/te/resource/Deserializer.h
)
target_link_libraries(te_resource PRIVATE te_core te_object)

# Always build 013's own tests (TENENGINE_SKIP_DEPENDENCY_TESTS only skips upstream 001/002 tests).
add_subdirectory(tests)
