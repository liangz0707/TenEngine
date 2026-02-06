# 001-Core library; C++17, contract-first (specs/_contracts/001-core-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(CORE_SOURCES
  src/alloc.cpp
  src/engine.cpp
  src/thread.cpp
  src/platform.cpp
  src/log.cpp
  src/math.cpp
  src/containers.cpp
  src/module_load.cpp
)

# Header files (for Visual Studio project view)
set(CORE_HEADERS
  include/te/core/alloc.h
  include/te/core/check.h
  include/te/core/containers.h
  include/te/core/engine.h
  include/te/core/log.h
  include/te/core/math.h
  include/te/core/module_load.h
  include/te/core/platform.h
  include/te/core/thread.h
)

add_library(te_core STATIC
  ${CORE_SOURCES}
  ${CORE_HEADERS}
)

target_include_directories(te_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Set output directory for Visual Studio
set_target_properties(te_core PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${CORE_SOURCES})
source_group("Header Files" FILES ${CORE_HEADERS})

# Organize files in Visual Studio project view
source_group("Source Files" FILES ${CORE_SOURCES})
source_group("Header Files" FILES ${CORE_HEADERS})

# Set header files as PUBLIC_HEADER so they appear in Visual Studio
set_target_properties(te_core PROPERTIES
  PUBLIC_HEADER "${CORE_HEADERS}"
)

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
  add_subdirectory(tests)
endif()
