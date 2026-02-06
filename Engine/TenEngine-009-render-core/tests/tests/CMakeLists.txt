cmake_minimum_required(VERSION 3.16)

# Unit tests
add_executable(test_render_core unit/test_render_core.cpp)
target_link_libraries(test_render_core PRIVATE te_rendercore)
add_test(NAME RenderCoreUnitTests COMMAND test_render_core)

# Contract tests (RHI integration smoke tests)
add_executable(test_render_core_contract contract/test_rhi_integration.cpp)
target_link_libraries(test_render_core_contract PRIVATE te_rendercore)
add_test(NAME RenderCoreContractTests COMMAND test_render_core_contract)