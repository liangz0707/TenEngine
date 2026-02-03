cmake_minimum_required(VERSION 3.16)

# Unit tests (T020)
add_executable(test_render_core unit/test_render_core.cpp)
target_link_libraries(test_render_core PRIVATE te_rendercore)
add_test(NAME RenderCoreUnitTests COMMAND test_render_core)

# Contract tests (T021) — RHI integration when 008-RHI harness available
add_executable(test_rhi_integration contract/test_rhi_integration.cpp)
target_link_libraries(test_rhi_integration PRIVATE te_rendercore)
add_test(NAME RenderCoreContractTests COMMAND test_rhi_integration)
