# TenEngine-017-uicore: Layout, DrawList, HitTest, Focus
# Depends: 001-core, 003-application, 006-input, imgui
# Contract: specs/_contracts/017-uicore-public-api.md

cmake_minimum_required(VERSION 3.16)
project(te_uicore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" CACHE PATH "TenEngine helpers")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("017-uicore" OUT_DEPS MY_DEPS)

set(UICORE_SOURCES
  src/Layout.cpp
  src/DrawList.cpp
  src/HitTest.cpp
  src/Focus.cpp
)

set(UICORE_HEADERS
  include/te/uicore/Layout.h
  include/te/uicore/DrawList.h
  include/te/uicore/HitTest.h
  include/te/uicore/Focus.h
)

add_library(te_uicore STATIC
  ${UICORE_SOURCES}
  ${UICORE_HEADERS}
)

target_include_directories(te_uicore PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(te_uicore PRIVATE ${MY_DEPS})

# ImGui (core + Win32 + DX11, added by root when TENENGINE_BUILD_EDITOR)
if(TARGET imgui_backends)
  target_link_libraries(te_uicore PRIVATE imgui_backends)
endif()

if(WIN32)
  target_link_libraries(te_uicore PRIVATE d3d11)
endif()

set_target_properties(te_uicore PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

source_group("Source Files" FILES ${UICORE_SOURCES})
source_group("Header Files" FILES ${UICORE_HEADERS})
set_target_properties(te_uicore PROPERTIES PUBLIC_HEADER "${UICORE_HEADERS}")
