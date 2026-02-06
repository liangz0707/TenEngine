# TenEngine-002-object: build per docs/engine-build-module-convention.md
# Dependency: 001-core (source by default: ../TenEngine-001-core)

cmake_minimum_required(VERSION 3.16)
project(TenEngine-002-object LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CTest at project root so add_test() from helpers are discovered (must be before tests).
enable_testing()

# Use own cmake (convention: downstream may use sibling upstream's cmake; we use own so scripts exist without 001)
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "TenEngine helpers and dependency map")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Resolve direct dependencies (002-object depends on 001-core per TenEngineModuleDependencies.cmake).
# build 测试时不引入上游模块的测试工程（docs/engine-build-module-convention.md §1）
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("002-object" OUT_DEPS MY_DEPS)

# Header files (for Visual Studio project view)
set(OBJECT_HEADERS
  include/te/object/TypeId.h
  include/te/object/TypeRegistry.h
  include/te/object/Guid.h
  include/te/object/Serializer.h
  include/te/object/PropertyBag.h
  include/te/object/VersionMigration.h
)

# Library: te_object (contract: specs/_contracts/002-object-public-api.md)
# Target name must match ABI: te_object (not TenEngine_002_object)
add_library(te_object STATIC
  src/TypeRegistry.cpp
  src/Guid.cpp
  src/BinarySerializer.cpp
  src/JSONSerializer.cpp
  src/XMLSerializer.cpp
  src/PropertyBag.cpp
  src/SerializerHelpers.cpp
  ${OBJECT_HEADERS}
)

target_include_directories(te_object PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(te_object PRIVATE ${MY_DEPS})

# Set output directory for Visual Studio
set_target_properties(te_object PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES 
  src/TypeRegistry.cpp
  src/Guid.cpp
  src/BinarySerializer.cpp
  src/JSONSerializer.cpp
  src/XMLSerializer.cpp
  src/PropertyBag.cpp
  src/SerializerHelpers.cpp
)
source_group("Header Files" FILES ${OBJECT_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_object PROPERTIES
  PUBLIC_HEADER "${OBJECT_HEADERS}"
)

# Tests: only link te_object (deps transitive per convention)
# Note: Tests are commented out until module is implemented
# tenengine_add_module_test(NAME te_object_TypeRegistry_test MODULE_TARGET te_object SOURCES tests/unit/TypeRegistry_test.cpp ENABLE_CTEST)
# tenengine_add_module_test(NAME te_object_Serializer_roundtrip_test MODULE_TARGET te_object SOURCES tests/unit/Serializer_roundtrip_test.cpp ENABLE_CTEST)
# tenengine_add_module_test(NAME te_object_PropertyBag_test MODULE_TARGET te_object SOURCES tests/unit/PropertyBag_test.cpp ENABLE_CTEST)
# tenengine_add_module_test(NAME te_object_CreateInstance_test MODULE_TARGET te_object SOURCES tests/unit/CreateInstance_test.cpp ENABLE_CTEST)
