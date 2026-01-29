# TenEngine-002-object: build per docs/build-module-convention.md
# Dependency: 001-core (source by default: ../TenEngine-001-core, or DLL via TENENGINE_001_CORE_USE_SOURCE=OFF)

cmake_minimum_required(VERSION 3.16)
project(TenEngine-002-object LANGUAGES CXX VERSION 0.1.0)

# Enable CTest at project root so add_test() from helpers are discovered (must be before tests).
enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use own cmake (convention: downstream may use sibling upstream's cmake; we use own so scripts exist without 001)
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "TenEngine helpers and dependency map")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Resolve direct dependencies (002-object depends on 001-core per TenEngineModuleDependencies.cmake).
# When TENENGINE_002_OBJECT_STANDALONE=ON, skip resolving 001 (build without 001; plan: "不引入外部库" for 001).
set(TENENGINE_002_OBJECT_STANDALONE OFF CACHE BOOL "Build 002-object without linking 001-core (no external deps)")
if(TENENGINE_002_OBJECT_STANDALONE)
  set(MY_DEPS "")
else()
  tenengine_resolve_my_dependencies("002-object" OUT_DEPS MY_DEPS)
endif()

# Library: te_object (contract: specs/_contracts/002-object-public-api.md)
add_library(te_object STATIC
  modules/object/src/CoreMemory.cpp
  modules/object/src/TypeRegistry.cpp
  modules/object/src/VersionMigration.cpp
  modules/object/src/Serializer.cpp
  modules/object/src/PropertyBag.cpp
)
target_include_directories(te_object PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/modules/object/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(te_object PRIVATE ${MY_DEPS})

# Tests: only link te_object (deps transitive per convention)
tenengine_add_module_test(NAME te_object_TypeRegistry_test MODULE_TARGET te_object SOURCES modules/object/tests/unit/TypeRegistry_test.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_object_Serializer_roundtrip_test MODULE_TARGET te_object SOURCES modules/object/tests/unit/Serializer_roundtrip_test.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_object_PropertyBag_test MODULE_TARGET te_object SOURCES modules/object/tests/unit/PropertyBag_test.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_object_CreateInstance_test MODULE_TARGET te_object SOURCES modules/object/tests/unit/CreateInstance_test.cpp ENABLE_CTEST)
