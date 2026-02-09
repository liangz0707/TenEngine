/** @file test_device_resource_manager.cpp
 *  030-DeviceResourceManager unit tests.
 */
#include <te/deviceresource/DeviceResourceManager.h>
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/core/log.h>
#include <cassert>
#include <cstring>

using namespace te::deviceresource;

namespace {

// Test parameter validation for CreateDeviceTexture
void TestCreateDeviceTextureValidation() {
  // Test null pixel data
  te::rhi::TextureDesc desc{};
  desc.width = 64;
  desc.height = 64;
  desc.depth = 1;
  desc.format = 0; // RGBA8
  
  te::rhi::ITexture* result = DeviceResourceManager::CreateDeviceTexture(
      nullptr, 0, desc, nullptr);
  assert(result == nullptr);
  
  // Test invalid texture dimensions
  te::rhi::TextureDesc invalidDesc{};
  invalidDesc.width = 0;
  invalidDesc.height = 64;
  
  result = DeviceResourceManager::CreateDeviceTexture(
      nullptr, 0, invalidDesc, nullptr);
  assert(result == nullptr);
}

// Test parameter validation for CreateDeviceBuffer
void TestCreateDeviceBufferValidation() {
  // Test null data
  te::rhi::BufferDesc desc{};
  desc.size = 1024;
  desc.usage = static_cast<uint32_t>(te::rhi::BufferUsage::Vertex);
  
  te::rhi::IBuffer* result = DeviceResourceManager::CreateDeviceBuffer(
      nullptr, 0, desc, nullptr);
  assert(result == nullptr);
  
  // Test invalid buffer description
  te::rhi::BufferDesc invalidDesc{};
  invalidDesc.size = 0;
  invalidDesc.usage = static_cast<uint32_t>(te::rhi::BufferUsage::Vertex);
  
  result = DeviceResourceManager::CreateDeviceBuffer(
      nullptr, 0, invalidDesc, nullptr);
  assert(result == nullptr);
}

// Test parameter validation for UpdateDeviceTexture
void TestUpdateDeviceTextureValidation() {
  te::rhi::TextureDesc desc{};
  desc.width = 64;
  desc.height = 64;
  desc.depth = 1;
  desc.format = 0;
  
  // Test null texture
  bool result = DeviceResourceManager::UpdateDeviceTexture(
      nullptr, nullptr, nullptr, 0, desc);
  assert(result == false);
  
  // Test invalid texture description
  te::rhi::TextureDesc invalidDesc{};
  invalidDesc.width = 0;
  invalidDesc.height = 64;
  
  result = DeviceResourceManager::UpdateDeviceTexture(
      nullptr, nullptr, nullptr, 0, invalidDesc);
  assert(result == false);
}

// Test CleanupDevice with null device (should not crash)
void TestCleanupDeviceNull() {
  DeviceResourceManager::CleanupDevice(nullptr);
  // Should not crash
}

// Test DestroyDeviceTexture with null parameters (should not crash)
void TestDestroyDeviceTextureNull() {
  DeviceResourceManager::DestroyDeviceTexture(nullptr, nullptr);
  // Should not crash
}

// Test DestroyDeviceBuffer with null parameters (should not crash)
void TestDestroyDeviceBufferNull() {
  DeviceResourceManager::DestroyDeviceBuffer(nullptr, nullptr);
  // Should not crash
}

}  // namespace

int main() {
  // Test parameter validation
  TestCreateDeviceTextureValidation();
  TestCreateDeviceBufferValidation();
  TestUpdateDeviceTextureValidation();
  
  // Test null parameter handling (should not crash)
  TestCleanupDeviceNull();
  TestDestroyDeviceTextureNull();
  TestDestroyDeviceBufferNull();
  
  // Note: Full integration tests require a valid RHI device,
  // which is typically provided by the test framework or test harness.
  // These tests focus on parameter validation and null safety.
  
  return 0;
}
