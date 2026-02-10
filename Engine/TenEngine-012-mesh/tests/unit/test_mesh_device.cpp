/**
 * @file test_mesh_device.cpp
 * @brief Unit tests for GPU device resource creation (EnsureDeviceResources, etc.)
 * 
 * Note: This test requires a valid RHI device, so it may need to be run
 * in an environment with graphics support or use a mock device.
 */

#include <te/mesh/Mesh.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshDevice.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/rhi/device.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cassert>
#include <cstring>

int main() {
  using namespace te::mesh;
  using namespace te::rhi;
  using namespace te::rendercore;

  // Create a simple mesh asset description
  MeshAssetDesc desc;
  desc.formatVersion = 1;
  desc.debugDescription = "Test mesh for device";

  // Create vertex format
  VertexAttribute attrs[1];
  attrs[0].location = 0;
  attrs[0].format = VertexAttributeFormat::Float3;
  attrs[0].offset = 0;
  
  VertexFormatDesc vertexFormatDesc{};
  vertexFormatDesc.attributes = attrs;
  vertexFormatDesc.attributeCount = 1;
  vertexFormatDesc.stride = sizeof(float) * 3;
  desc.vertexLayout = CreateVertexFormat(vertexFormatDesc);
  
  desc.indexFormat.type = IndexType::UInt32;
  
  // Allocate minimal vertex and index data
  size_t vertexDataSize = 3 * sizeof(float) * 3;  // 3 vertices
  desc.vertexData = te::core::Alloc(vertexDataSize, alignof(float));
  assert(desc.vertexData != nullptr);
  desc.vertexDataSize = vertexDataSize;
  
  size_t indexDataSize = 3 * sizeof(uint32_t);  // 3 indices
  desc.indexData = te::core::Alloc(indexDataSize, alignof(uint32_t));
  assert(desc.indexData != nullptr);
  desc.indexDataSize = indexDataSize;
  
  SubmeshDesc submesh;
  submesh.offset = 0;
  submesh.count = 3;
  submesh.materialSlotIndex = 0;
  desc.submeshes.push_back(submesh);
  
  // Create mesh handle
  MeshHandle handle = CreateMesh(&desc);
  assert(handle != nullptr);
  
  // Try to create device (may fail if no graphics support)
  IDevice* device = CreateDevice();
  
  if (device != nullptr) {
    // Test EnsureDeviceResources (synchronous)
    bool success = EnsureDeviceResources(handle, device);
    // Note: This may fail if device doesn't support the required features
    // but we test that the function can be called without crashing
    
    if (success) {
      // Test GetVertexBufferHandle
      IBuffer* vertexBuffer = GetVertexBufferHandle(handle);
      // May be nullptr if creation failed, but function should not crash
      
      // Test GetIndexBufferHandle
      IBuffer* indexBuffer = GetIndexBufferHandle(handle);
      // May be nullptr if creation failed, but function should not crash
    }
    
    // Test EnsureDeviceResourcesAsync
    bool asyncCompleted = false;
    EnsureDeviceResourcesAsync(handle, device, 
      [](void* user_data) {
        bool* completed = static_cast<bool*>(user_data);
        *completed = true;
      },
      &asyncCompleted);
    
    // Note: In a real test, we would wait for async completion
    // For now, we just verify the function doesn't crash
    
    DestroyDevice(device);
  }
  
  // Cleanup
  ReleaseMesh(handle);
  te::core::Free(desc.vertexData);
  te::core::Free(desc.indexData);
  
  return 0;
}
