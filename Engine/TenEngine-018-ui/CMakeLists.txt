# TenEngine-018-ui: Canvas, Widget, DockablePanel, DockArea
# Depends: 017-uicore
# Contract: specs/_contracts/018-ui-public-api.md

cmake_minimum_required(VERSION 3.16)
project(te_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("018-ui" OUT_DEPS MY_DEPS)

set(UI_SOURCES
  src/Canvas.cpp
)

set(UI_HEADERS
  include/te/ui/Canvas.h
)

add_library(te_ui STATIC
  ${UI_SOURCES}
  ${UI_HEADERS}
)

target_include_directories(te_ui PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(te_ui PRIVATE ${MY_DEPS})

set_target_properties(te_ui PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

source_group("Source Files" FILES ${UI_SOURCES})
source_group("Header Files" FILES ${UI_HEADERS})
