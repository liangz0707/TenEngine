# TenEngine-002-object (contract: specs/_contracts/002-object-public-api.md, ABI: 002-object-ABI.md)
# Build root: run cmake from this directory (Engine/TenEngine-002-object) or repo root; 001-core via source (../TenEngine-001-core or TENENGINE_001_CORE_DIR).
# All submodules use SOURCE; do not run cmake until build root is confirmed per docs/engine-build-module-convention.md §3.

cmake_minimum_required(VERSION 3.16)
project(TenEngine-002-object LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

tenengine_resolve_my_dependencies("002-object" OUT_DEPS MY_DEPS)

set(TE_OBJECT_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(te_object STATIC
  src/CoreMemory.cpp
  src/TypeRegistry.cpp
  src/VersionMigration.cpp
  src/Serializer.cpp
  src/PropertyBag.cpp
)

target_include_directories(te_object PUBLIC
  $<BUILD_INTERFACE:${TE_OBJECT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(te_object PRIVATE ${MY_DEPS})

target_sources(te_object PUBLIC
  ${TE_OBJECT_INCLUDE_DIR}/te/object/TypeId.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/TypeDescriptor.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/PropertyDescriptor.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/SerializedBuffer.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/ObjectRef.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/Guid.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/TypeRegistry.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/VersionMigration.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/Serializer.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/PropertyBag.hpp
  ${TE_OBJECT_INCLUDE_DIR}/te/object/detail/CoreMemory.hpp
)

tenengine_add_module_test(NAME te_object_TypeRegistry_test MODULE_TARGET te_object
  SOURCES tests/unit/TypeRegistry_test.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_object_CreateInstance_test MODULE_TARGET te_object
  SOURCES tests/unit/CreateInstance_test.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_object_Serializer_roundtrip_test MODULE_TARGET te_object
  SOURCES tests/unit/Serializer_roundtrip_test.cpp ENABLE_CTEST)
tenengine_add_module_test(NAME te_object_PropertyBag_test MODULE_TARGET te_object
  SOURCES tests/unit/PropertyBag_test.cpp ENABLE_CTEST)
