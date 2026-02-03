# 009-RenderCore — no placeholders; upstream 008-RHI required (source).
# 008-RHI pulls in 001-Core via TenEngineHelpers; we only add_subdirectory(008-RHI).
# Per docs/engine-build-module-convention.md and docs/agent-build-guide.md.
# ABI: te::rendercore namespace, te_rendercore target (specs/_contracts/009-rendercore-ABI.md)

cmake_minimum_required(VERSION 3.16)
project(te_rendercore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Resolve upstream 008-RHI (FATAL_ERROR if missing); 001-Core resolved by 008-RHI ---
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/ResolveUpstream.cmake)

# --- Add 008-RHI from source (it adds 001-Core via TenEngineHelpers; no duplicate add) ---
set(BUILD_TESTS OFF)
set(TENENGINE_BUILD_TESTS OFF)
add_subdirectory(${009_RENDERCORE_RHI_DIR} ${CMAKE_BINARY_DIR}/_deps/008-rhi EXCLUDE_FROM_ALL)

# --- This module library (te_rendercore) ---
add_library(te_rendercore STATIC
    # Sources
    src/render_core/resource_desc.cpp
    src/render_core/uniform_layout.cpp
    src/render_core/pass_protocol.cpp
    src/render_core/uniform_buffer.cpp
    # Headers (for VS/IDE project visibility)
    include/te/rendercore/api.hpp
    include/te/rendercore/pass_protocol.hpp
    include/te/rendercore/resource_desc.hpp
    include/te/rendercore/types.hpp
    include/te/rendercore/uniform_buffer.hpp
    include/te/rendercore/uniform_layout.hpp
)

target_include_directories(te_rendercore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Group headers in VS Solution Explorer
source_group("Header Files\\te\\rendercore" FILES
    include/te/rendercore/api.hpp
    include/te/rendercore/pass_protocol.hpp
    include/te/rendercore/resource_desc.hpp
    include/te/rendercore/types.hpp
    include/te/rendercore/uniform_buffer.hpp
    include/te/rendercore/uniform_layout.hpp
)
source_group("Source Files" FILES
    src/render_core/resource_desc.cpp
    src/render_core/uniform_layout.cpp
    src/render_core/pass_protocol.cpp
    src/render_core/uniform_buffer.cpp
)

# --- Link upstream: 008-RHI exports te_rhi (and links te_core); no placeholder ---
if(NOT TARGET te_rhi)
    message(FATAL_ERROR "009-RenderCore: te_rhi target not found after add_subdirectory(008-RHI). Check upstream CMakeLists.txt.")
endif()

target_link_libraries(te_rendercore
    PUBLIC te_rhi
)

# --- Tests (only this module's tests) ---
option(TENENGINE_RENDERCORE_BUILD_TESTS "Build 009-RenderCore unit/contract tests" ON)
if(TENENGINE_RENDERCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
