cmake_minimum_required(VERSION 3.16)
project(te_rendercore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Resolve 008-RHI dependency (must be built from source per build convention)
if(NOT TARGET te_rhi)
    set(TE_RHI_SOURCE_DIR "")
    if(DEFINED TENENGINE_RHI_DIR)
        set(TE_RHI_SOURCE_DIR "${TENENGINE_RHI_DIR}")
    elseif(DEFINED ENV{TENENGINE_RHI_DIR})
        set(TE_RHI_SOURCE_DIR "$ENV{TENENGINE_RHI_DIR}")
    elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-008-rhi/CMakeLists.txt")
        set(TE_RHI_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-008-rhi")
    endif()

    if(TE_RHI_SOURCE_DIR STREQUAL "")
        message(FATAL_ERROR "te_rendercore requires the 008-RHI module. "
            "Set TENENGINE_RHI_DIR or place TenEngine-008-rhi adjacent to this worktree.")
    endif()

    message(STATUS "Adding 008-RHI from ${TE_RHI_SOURCE_DIR}")
    add_subdirectory("${TE_RHI_SOURCE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/008-rhi")
endif()

# Module library (implementation + ABI headers)
add_library(te_rendercore STATIC
    src/render_core/types.hpp
    src/render_core/resource_desc.hpp
    src/render_core/resource_desc.cpp
    src/render_core/shader_params.hpp
    src/render_core/shader_params.cpp
    src/render_core/pass_protocol.hpp
    src/render_core/pass_protocol.cpp
    src/render_core/uniform_buffer.hpp
    src/render_core/uniform_buffer.cpp
    src/render_core/uniform_layout.cpp
    src/render_core/api.hpp
)

target_include_directories(te_rendercore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(te_rendercore
    PUBLIC te_rhi
)

# Unit tests (optional)
option(TE_RENDERCORE_BUILD_TESTS "Build unit tests" OFF)
if(TE_RENDERCORE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
