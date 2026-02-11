# 008-RHI library; C++17, contract-first (specs/_contracts/008-rhi-public-api.md).
cmake_minimum_required(VERSION 3.16)
project(te_rhi LANGUAGES CXX)

enable_testing()
set(CMAKE_CXX_STANDARD 17)

# Backend and debug options (plan Phase 2: T004, T008)
option(TE_RHI_VULKAN "Enable Vulkan backend" ON)
option(TE_RHI_D3D12 "Enable D3D12 backend (Windows)" ON)
option(TE_RHI_D3D11 "Enable D3D11 backend (Windows)" OFF)
option(TE_RHI_METAL "Enable Metal backend (macOS)" OFF)
option(TE_RHI_VALIDATION "Enable Vulkan Validation Layer" OFF)
option(TE_RHI_DEBUG_LAYER "Enable D3D12 Debug Layer" OFF)

set(TENENGINE_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../TenEngine-001-core/cmake")
include(${TENENGINE_CMAKE_DIR}/TenEngineHelpers.cmake)
tenengine_resolve_my_dependencies("008-rhi" OUT_DEPS MY_DEPS)

# Third-party: volk + vulkan-headers when Vulkan backend enabled (T005)
if(TE_RHI_VULKAN)
  include(FetchContent)
  FetchContent_Declare(volk
    GIT_REPOSITORY https://github.com/zeux/volk.git
    GIT_TAG        1.2.162
  )
  FetchContent_Declare(Vulkan-Headers
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG        v1.3.280
  )
  set(VOLK_INSTALL OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(volk Vulkan-Headers)
endif()

set(TE_RHI_SOURCES
  src/device/device.cpp
  src/command_list/command_list.cpp
  src/sync/sync.cpp
)
if(TE_RHI_VULKAN)
  list(APPEND TE_RHI_SOURCES src/vulkan/device_vulkan.cpp)
endif()
if(TE_RHI_D3D12 AND WIN32)
  list(APPEND TE_RHI_SOURCES src/d3d12/device_d3d12.cpp)
endif()
if(TE_RHI_D3D11 AND WIN32)
  list(APPEND TE_RHI_SOURCES src/d3d11/device_d3d11.cpp)
endif()
if(TE_RHI_METAL AND APPLE)
  list(APPEND TE_RHI_SOURCES src/metal/device_metal.mm)
endif()

set(TE_RHI_HEADERS
  include/te/rhi/backend_d3d11.hpp
  include/te/rhi/backend_d3d12.hpp
  include/te/rhi/backend_metal.hpp
  include/te/rhi/backend_vulkan.hpp
  include/te/rhi/command_list.hpp
  include/te/rhi/descriptor_set.hpp
  include/te/rhi/device.hpp
  include/te/rhi/pso.hpp
  include/te/rhi/queue.hpp
  include/te/rhi/raytracing.hpp
  include/te/rhi/resources.hpp
  include/te/rhi/swapchain.hpp
  include/te/rhi/sync.hpp
  include/te/rhi/types.hpp
)

add_library(te_rhi STATIC ${TE_RHI_SOURCES} ${TE_RHI_HEADERS})
target_link_libraries(te_rhi PRIVATE ${MY_DEPS})
target_include_directories(te_rhi PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)
# Backend/validation compile definitions (T004, T006, T007, T008)
if(TE_RHI_VULKAN)
  target_compile_definitions(te_rhi PRIVATE TE_RHI_VULKAN=1)
endif()
if(TE_RHI_D3D12)
  target_compile_definitions(te_rhi PRIVATE TE_RHI_D3D12=1)
endif()
if(TE_RHI_D3D11)
  target_compile_definitions(te_rhi PRIVATE TE_RHI_D3D11=1)
endif()
if(TE_RHI_METAL)
  target_compile_definitions(te_rhi PRIVATE TE_RHI_METAL=1)
endif()
if(TE_RHI_VALIDATION)
  target_compile_definitions(te_rhi PRIVATE TE_RHI_VALIDATION=1)
endif()
if(TE_RHI_DEBUG_LAYER)
  target_compile_definitions(te_rhi PRIVATE TE_RHI_DEBUG_LAYER=1)
endif()
if(TE_RHI_VULKAN AND TARGET volk)
  target_link_libraries(te_rhi PRIVATE volk)
endif()
if(TE_RHI_D3D12 AND WIN32)
  target_link_libraries(te_rhi PRIVATE d3d12 dxgi)
endif()
if(TE_RHI_D3D11 AND WIN32)
  target_link_libraries(te_rhi PRIVATE d3d11 dxgi)
endif()
if(TE_RHI_METAL AND APPLE)
  target_link_libraries(te_rhi PRIVATE "-framework Metal" "-framework QuartzCore" "-framework Foundation")
endif()

source_group("Header Files" FILES ${TE_RHI_HEADERS})
set_target_properties(te_rhi PROPERTIES PUBLIC_HEADER "${TE_RHI_HEADERS}")

if(NOT TENENGINE_SKIP_DEPENDENCY_TESTS)
tenengine_add_module_test(
  NAME te_rhi_test
  MODULE_TARGET te_rhi
  SOURCES tests/device_create.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME te_rhi_command_list_test
  MODULE_TARGET te_rhi
  SOURCES tests/command_list_submit.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME te_rhi_resources_test
  MODULE_TARGET te_rhi
  SOURCES tests/resources_create.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME te_rhi_pso_test
  MODULE_TARGET te_rhi
  SOURCES tests/pso_create.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME te_rhi_sync_test
  MODULE_TARGET te_rhi
  SOURCES tests/sync_fence_semaphore.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME te_rhi_swapchain_test
  MODULE_TARGET te_rhi
  SOURCES tests/swapchain_create.cpp
  ENABLE_CTEST
)
tenengine_add_module_test(
  NAME te_rhi_buffer_update_uniform_bind_test
  MODULE_TARGET te_rhi
  SOURCES tests/buffer_update_uniform_bind.cpp
  ENABLE_CTEST
)
endif()