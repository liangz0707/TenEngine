cmake_minimum_required(VERSION 3.16)
project(TenEngine_RenderCore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options for dependency import (source by default per plan.md)
option(TENENGINE_CORE_USE_SOURCE "Build 001-Core from source" ON)
option(TENENGINE_RHI_USE_SOURCE "Build 008-RHI from source" ON)

# Module library
add_library(TenEngine_RenderCore STATIC
    src/render_core/types.hpp
    src/render_core/resource_desc.hpp
    src/render_core/resource_desc.cpp
    src/render_core/shader_params.hpp
    src/render_core/shader_params.cpp
    src/render_core/pass_protocol.hpp
    src/render_core/pass_protocol.cpp
    src/render_core/uniform_buffer.hpp
    src/render_core/uniform_buffer.cpp
    src/render_core/api.hpp
)

target_include_directories(TenEngine_RenderCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Upstream dependencies (source import per plan.md)
# When integrated with main TenEngine repo, link to TenEngine_Core and TenEngine_RHI targets.
# For standalone build, these are placeholder interface libraries.
if(NOT TARGET TenEngine_Core)
    add_library(TenEngine_Core INTERFACE)
    message(STATUS "TenEngine_Core not found, using placeholder interface. Integrate with main repo for full build.")
endif()

if(NOT TARGET TenEngine_RHI)
    add_library(TenEngine_RHI INTERFACE)
    message(STATUS "TenEngine_RHI not found, using placeholder interface. Integrate with main repo for full build.")
endif()

target_link_libraries(TenEngine_RenderCore
    PUBLIC TenEngine_Core
    PUBLIC TenEngine_RHI
)

# Unit tests (optional)
option(TENENGINE_RENDERCORE_BUILD_TESTS "Build unit tests" OFF)
if(TENENGINE_RENDERCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
