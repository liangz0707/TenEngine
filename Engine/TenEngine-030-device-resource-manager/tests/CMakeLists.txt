# 030-DeviceResourceManager tests
cmake_minimum_required(VERSION 3.16)

# 本模块测试始终加入（不受 TENENGINE_SKIP_DEPENDENCY_TESTS 影响）
# TENENGINE_SKIP_DEPENDENCY_TESTS 只影响上游依赖模块的测试
add_subdirectory(unit)
