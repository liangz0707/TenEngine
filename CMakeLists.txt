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

# Exclude upstream module tests from default builds (per build convention)
set(TE_CORE_TEST_TARGETS
    test_alloc
    test_thread
    test_platform
    test_log
    test_math
    test_containers
    test_module_load
    test_engine
    test_check
)
foreach(test_target IN LISTS TE_CORE_TEST_TARGETS)
    if(TARGET ${test_target})
        set_target_properties(${test_target} PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()
endforeach()

set(TE_RHI_TEST_TARGETS
    te_rhi_test
    te_rhi_command_list_test
    te_rhi_resources_test
    te_rhi_pso_test
    te_rhi_sync_test
    te_rhi_swapchain_test
    te_rhi_buffer_update_uniform_bind_test
)
foreach(test_target IN LISTS TE_RHI_TEST_TARGETS)
    if(TARGET ${test_target})
        set_target_properties(${test_target} PROPERTIES
            EXCLUDE_FROM_ALL TRUE
            EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    endif()
endforeach()

set(TE_RENDERCORE_PUBLIC_HEADERS
    include/te/rendercore/api.hpp
    include/te/rendercore/types.hpp
    include/te/rendercore/resource_desc.hpp
    include/te/rendercore/uniform_layout.hpp
    include/te/rendercore/shader_reflection.hpp
    include/te/rendercore/pass_protocol.hpp
    include/te/rendercore/uniform_buffer.hpp
)

set(TE_RENDERCORE_INTERNAL_HEADERS
    src/render_core/types.hpp
    src/render_core/resource_desc.hpp
    src/render_core/shader_params.hpp
    src/render_core/pass_protocol.hpp
    src/render_core/uniform_buffer.hpp
    src/render_core/api.hpp
)

# Module library (implementation + ABI headers)
add_library(te_rendercore STATIC
    ${TE_RENDERCORE_PUBLIC_HEADERS}
    ${TE_RENDERCORE_INTERNAL_HEADERS}
    src/render_core/resource_desc.cpp
    src/render_core/shader_params.cpp
    src/render_core/pass_protocol.cpp
    src/render_core/uniform_buffer.cpp
    src/render_core/uniform_layout.cpp
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
