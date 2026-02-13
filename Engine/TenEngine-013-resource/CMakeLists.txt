# 013-Resource worktree (build convention: docs/engine-build-module-convention.md).
# 本 worktree 仅包含：001-Core（in-tree）+ 013-Resource；src/ 与 include/ 不得包含 004-Scene、019-PipelineCore 等它模块代码。
# C++17, contract-first (specs/_contracts/013-resource-ABI.md).
# Build root: worktree root. Dependencies: 001-core (in-tree), 002-object (resolve via TENENGINE_*_DIR when present).
# Note: 013 does NOT depend on 028-Texture to avoid circular dependency. 013 defines ITextureResource interface,
# and 028 implements it and registers Loader to 013 via registration mechanism.
cmake_minimum_required(VERSION 3.16)
project(TenEngine-013-resource LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# --- 001-Core: in-tree when built standalone; reuse te_core if already added (e.g. by 010) ---
if(NOT TARGET te_core)
  set(CORE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core")
  set(CORE_SOURCES
    ${CORE_SOURCE_DIR}/src/alloc.cpp
    ${CORE_SOURCE_DIR}/src/engine.cpp
    ${CORE_SOURCE_DIR}/src/thread.cpp
    ${CORE_SOURCE_DIR}/src/platform.cpp
    ${CORE_SOURCE_DIR}/src/log.cpp
    ${CORE_SOURCE_DIR}/src/math.cpp
    ${CORE_SOURCE_DIR}/src/containers.cpp
    ${CORE_SOURCE_DIR}/src/module_load.cpp
  )
  set(CORE_HEADERS
    ${CORE_SOURCE_DIR}/include/te/core/alloc.h
    ${CORE_SOURCE_DIR}/include/te/core/check.h
    ${CORE_SOURCE_DIR}/include/te/core/containers.h
    ${CORE_SOURCE_DIR}/include/te/core/engine.h
    ${CORE_SOURCE_DIR}/include/te/core/log.h
    ${CORE_SOURCE_DIR}/include/te/core/math.h
    ${CORE_SOURCE_DIR}/include/te/core/module_load.h
    ${CORE_SOURCE_DIR}/include/te/core/platform.h
    ${CORE_SOURCE_DIR}/include/te/core/thread.h
  )
  add_library(te_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
  target_include_directories(te_core PUBLIC ${CORE_SOURCE_DIR}/include)
  source_group("Source Files" FILES ${CORE_SOURCES})
  source_group("Header Files" FILES ${CORE_HEADERS})
endif()

# --- 013-Resource (depends on te_core; 002-object via TenEngineHelpers when TENENGINE_*_DIR set) ---
# Resource source files
set(RESOURCE_SOURCES
  src/ResourceManager.cpp
  src/Resource.cpp
  src/ResourceRepositoryConfig.cpp
  src/ResourceManifest.cpp
)

# Resource header files (for Visual Studio project view)
set(RESOURCE_HEADERS
  include/te/resource/EffectResource.h
  include/te/resource/MaterialResource.h
  include/te/resource/MeshResource.h
  include/te/resource/Resource.h
  include/te/resource/Resource.inl
  include/te/resource/ResourceId.h
  include/te/resource/ResourceManager.h
  include/te/resource/ResourceManifest.h
  include/te/resource/ResourceRepositoryConfig.h
  include/te/resource/ResourceTypes.h
  include/te/resource/ShaderResource.h
  include/te/resource/TerrainResource.h
  include/te/resource/TextureResource.h
)

add_library(te_resource STATIC
  ${RESOURCE_SOURCES}
  ${RESOURCE_HEADERS}
)
target_include_directories(te_resource PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(te_resource PRIVATE te_core)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${RESOURCE_SOURCES})
source_group("Header Files" FILES ${RESOURCE_HEADERS})

# Link 002-object if available (via TenEngineHelpers)
# Note: 001-core is already defined above (in-tree), so we only resolve 002-object
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/TenEngineHelpers.cmake)
# Mark 001-core as already added to prevent duplicate target
set_property(GLOBAL PROPERTY TENENGINE_ADDED_001-core TRUE)
# Resolve 002-object dependency only
tenengine_resolve_dependency("002-object" _te_object_target)
if(_te_object_target AND TARGET ${_te_object_target})
  target_link_libraries(te_resource PUBLIC ${_te_object_target})
endif()

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
