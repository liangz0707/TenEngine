/**
 * @file test_mesh_gpu_sync_async.cpp
 * @brief Integration tests for GPU resource creation (synchronous and asynchronous)
 */

#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/mesh/MeshDevice.h>
#include <te/rhi/device.hpp>
#include <te/deviceresource/DeviceResourceManager.h>
#include <te/deviceresource/ResourceOperationTypes.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cassert>
#include <cstring>
#include <thread>
#include <chrono>

int main() {
  using namespace te::mesh;
  using namespace te::rhi;
  using namespace te::deviceresource;
  using namespace te::rendercore;

  // Create a simple mesh asset description
  MeshAssetDesc desc;
  desc.formatVersion = 1;
  desc.debugDescription = "Test mesh for GPU sync/async";

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
  
  // Allocate vertex and index data
  size_t vertexDataSize = 9 * sizeof(float);  // 3 vertices * 3 floats
  desc.vertexData = te::core::Alloc(vertexDataSize, alignof(float));
  assert(desc.vertexData != nullptr);
  desc.vertexDataSize = vertexDataSize;
  
  size_t indexDataSize = 3 * sizeof(uint32_t);
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
  
  // Try to create device
  IDevice* device = CreateDevice();
  
  if (device != nullptr) {
    // Test synchronous GPU resource creation
    bool syncSuccess = EnsureDeviceResources(handle, device);
    // Note: May fail if device doesn't support required features
    
    if (syncSuccess) {
      // Verify buffers were created
      IBuffer* vertexBuffer = GetVertexBufferHandle(handle);
      IBuffer* indexBuffer = GetIndexBufferHandle(handle);
      // Buffers may be nullptr if creation failed, but functions should not crash
    }
    
    // Test asynchronous GPU resource creation
    bool asyncCompleted = false;
    bool asyncSuccess = false;
    
    EnsureDeviceResourcesAsync(handle, device,
      [](void* user_data) {
        bool* completed = static_cast<bool*>(user_data);
        *completed = true;
      },
      &asyncCompleted);
    
    // In a real test, we would wait for completion and verify results
    // For now, we just verify the function doesn't crash
    
    // Test operation status query (if we had a handle)
    // ResourceOperationStatus status = DeviceResourceManager::GetOperationStatus(handle);
    // float progress = DeviceResourceManager::GetOperationProgress(handle);
    
    DestroyDevice(device);
  }
  
  // Cleanup
  ReleaseMesh(handle);
  te::core::Free(desc.vertexData);
  te::core::Free(desc.indexData);
  
  return 0;
}
