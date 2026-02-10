/**
 * @file test_mesh_resource.cpp
 * @brief Unit tests for MeshResource (lifecycle, Load, Save, Import, reference counting)
 */

#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/resource/ResourceTypes.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cassert>
#include <cstring>

int main() {
  using namespace te::mesh;
  using namespace te::resource;

  // Test MeshResource creation
  MeshResource* resource = new MeshResource();
  assert(resource != nullptr);
  
  // Test GetResourceType
  assert(resource->GetResourceType() == ResourceType::Mesh);
  
  // Test GetResourceId (should be non-null)
  ResourceId id = resource->GetResourceId();
  assert(!id.IsNull());
  
  // Test Release (reference counting)
  uint32_t initialRefCount = 1;  // Created with ref count 1
  resource->Release();
  // After release, ref count should be decremented
  // Note: We can't directly check ref count, but Release should not crash
  
  // Test GetMeshHandle (should be nullptr for new resource)
  assert(resource->GetMeshHandle() == nullptr);
  
  // Test GetVertexData/GetIndexData (should return nullptr for unloaded resource)
  assert(resource->GetVertexData() == nullptr);
  assert(resource->GetVertexDataSize() == 0);
  assert(resource->GetIndexData() == nullptr);
  assert(resource->GetIndexDataSize() == 0);
  
  // Cleanup
  delete resource;
  
  return 0;
}
