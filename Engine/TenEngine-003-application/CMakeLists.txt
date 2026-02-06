# 003-Application library; C++17, contract-first (specs/_contracts/003-application-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_application LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable CTest at project root so add_test() from helpers are discovered (must be before tests).
enable_testing()

# Use upstream (001-core) cmake helpers for dependency resolution
# Convention: downstream may use sibling upstream's cmake
set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake" CACHE PATH "TenEngine helpers and dependency map")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)

# Resolve direct dependencies (003-application depends on 001-core per TenEngineModuleDependencies.cmake).
# build 测试时不引入上游模块的测试工程（docs/engine-build-module-convention.md §1）
set(TENENGINE_SKIP_DEPENDENCY_TESTS ON CACHE BOOL "Skip building dependency module tests")
tenengine_resolve_my_dependencies("003-application" OUT_DEPS MY_DEPS)

# Source files
set(APPLICATION_SOURCES
  src/Application.cpp
  src/EventQueue.cpp
  src/platform/PlatformAbstraction.cpp
)

# Header files (for Visual Studio project view)
set(APPLICATION_HEADERS
  include/te/application/Application.h
  include/te/application/Window.h
  include/te/application/Event.h
  include/te/application/MainLoop.h
  include/te/application/Platform.h
)

# Library: te_application (contract: specs/_contracts/003-application-public-api.md)
# Target name must match ABI: te_application (not TenEngine_003_application)
add_library(te_application STATIC
  ${APPLICATION_SOURCES}
  ${APPLICATION_HEADERS}
)

target_include_directories(te_application PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Link dependencies (use resolved dependencies from tenengine_resolve_my_dependencies)
target_link_libraries(te_application PRIVATE ${MY_DEPS})

# Platform-specific libraries
if(TE_PLATFORM_WINDOWS)
  target_link_libraries(te_application PRIVATE user32 gdi32)
endif()

# Set output directory for Visual Studio
set_target_properties(te_application PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${APPLICATION_SOURCES})
source_group("Header Files" FILES ${APPLICATION_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_application PROPERTIES
  PUBLIC_HEADER "${APPLICATION_HEADERS}"
)

# Platform-specific sources
if(TE_PLATFORM_WINDOWS)
  list(APPEND APPLICATION_SOURCES
    src/platform/windows/WindowsWindowPlatform.cpp
    src/platform/windows/WindowsEventPumpPlatform.cpp
  )
elseif(TE_PLATFORM_LINUX)
  # TODO: Add X11 platform implementations
  # list(APPEND APPLICATION_SOURCES
  #   src/platform/x11/X11WindowPlatform.cpp
  #   src/platform/x11/X11EventPumpPlatform.cpp
  # )
elseif(TE_PLATFORM_MACOS)
  # TODO: Add Cocoa platform implementations
  # list(APPEND APPLICATION_SOURCES
  #   src/platform/cocoa/CocoaWindowPlatform.mm
  #   src/platform/cocoa/CocoaEventPumpPlatform.mm
  # )
endif()

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  # TODO: Add tests subdirectory when tests are implemented
  # add_subdirectory(tests)
endif()
