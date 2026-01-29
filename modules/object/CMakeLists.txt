# 002-Object module: reflection, serialization, property system, type registry.
# Contract: specs/_contracts/002-object-public-api.md
# Depends on 001-Core (optional at configure; link when available).

cmake_minimum_required(VERSION 3.14)
project(te_object LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(te_object
  src/TypeRegistry.cpp
  src/VersionMigration.cpp
  src/Serializer.cpp
  src/PropertyBag.cpp
)
target_include_directories(te_object PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
# Optional: link 001-Core when present (contract: only use Core-declared types).
# if(TARGET te_core)
#   target_link_libraries(te_object PUBLIC te_core)
# endif()

enable_testing()
add_subdirectory(tests)
