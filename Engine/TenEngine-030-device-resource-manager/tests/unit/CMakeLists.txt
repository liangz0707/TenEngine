# 030-DeviceResourceManager unit tests
cmake_minimum_required(VERSION 3.16)

# Test executable
add_executable(te_deviceresource_test
  test_device_resource_manager.cpp
)

# Link to the library - dependencies are inherited via PUBLIC/PRIVATE interface
target_link_libraries(te_deviceresource_test PRIVATE te_deviceresource)

# Register test with CTest (enable_testing() is called in parent CMakeLists.txt)
add_test(NAME te_deviceresource_test COMMAND te_deviceresource_test)
