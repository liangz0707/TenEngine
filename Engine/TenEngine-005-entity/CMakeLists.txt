# 005-Entity library; C++17, contract-first (specs/_contracts/005-entity-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_entity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include CMake helpers for dependency resolution
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Source files
set(ENTITY_SOURCES
  src/Entity.cpp
  src/EntityManager.cpp
  src/ComponentRegistry.cpp
  src/ComponentRegistration.cpp
  src/System.cpp
)

# Header files (for Visual Studio project view)
set(ENTITY_HEADERS
  include/te/entity/Entity.h
  include/te/entity/EntityId.h
  include/te/entity/EntityManager.h
  include/te/entity/Component.h
  include/te/entity/ComponentQuery.h
  include/te/entity/ComponentRegistry.h
  include/te/entity/ComponentRegistration.h
  include/te/entity/System.h
)

add_library(te_entity STATIC
  ${ENTITY_SOURCES}
  ${ENTITY_HEADERS}
)

target_include_directories(te_entity PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Resolve dependencies: Core, Object, Scene
tenengine_resolve_my_dependencies("005-entity" OUT_DEPS _entity_deps)
target_link_libraries(te_entity PUBLIC ${_entity_deps})

# Set output directory for Visual Studio
set_target_properties(te_entity PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${ENTITY_SOURCES})
source_group("Header Files" FILES ${ENTITY_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_entity PROPERTIES
  PUBLIC_HEADER "${ENTITY_HEADERS}"
)

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
