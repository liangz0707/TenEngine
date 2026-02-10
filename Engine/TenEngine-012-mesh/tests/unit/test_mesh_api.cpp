/**
 * @file test_mesh_api.cpp
 * @brief Unit tests for mesh API (CreateMesh, ReleaseMesh, GetSubmesh, etc.)
 */

#include <te/mesh/Mesh.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cassert>
#include <cstring>

int main() {
  using namespace te::mesh;
  using namespace te::rendercore;

  // Create a simple mesh asset description
  MeshAssetDesc desc;
  desc.formatVersion = 1;
  desc.debugDescription = "Test mesh";

  // Create vertex format (position + normal + UV)
  VertexAttribute attrs[3];
  uint32_t offset = 0;
  
  attrs[0].location = 0;
  attrs[0].format = VertexAttributeFormat::Float3;
  attrs[0].offset = offset;
  offset += sizeof(float) * 3;
  
  attrs[1].location = 1;
  attrs[1].format = VertexAttributeFormat::Float3;
  attrs[1].offset = offset;
  offset += sizeof(float) * 3;
  
  attrs[2].location = 2;
  attrs[2].format = VertexAttributeFormat::Float2;
  attrs[2].offset = offset;
  offset += sizeof(float) * 2;
  
  VertexFormatDesc vertexFormatDesc{};
  vertexFormatDesc.attributes = attrs;
  vertexFormatDesc.attributeCount = 3;
  vertexFormatDesc.stride = offset;
  desc.vertexLayout = CreateVertexFormat(vertexFormatDesc);
  
  // Create index format
  desc.indexFormat.type = IndexType::UInt32;
  
  // Allocate vertex data (3 vertices)
  size_t vertexCount = 3;
  size_t vertexDataSize = vertexCount * vertexFormatDesc.stride;
  desc.vertexData = te::core::Alloc(vertexDataSize, alignof(float));
  assert(desc.vertexData != nullptr);
  desc.vertexDataSize = vertexDataSize;
  
  // Fill vertex data with test values
  float* vertexPtr = static_cast<float*>(desc.vertexData);
  for (size_t i = 0; i < vertexCount; ++i) {
    // Position
    vertexPtr[0] = static_cast<float>(i);
    vertexPtr[1] = static_cast<float>(i + 1);
    vertexPtr[2] = static_cast<float>(i + 2);
    vertexPtr += 3;
    // Normal
    vertexPtr[0] = 0.0f;
    vertexPtr[1] = 1.0f;
    vertexPtr[2] = 0.0f;
    vertexPtr += 3;
    // UV
    vertexPtr[0] = static_cast<float>(i) * 0.5f;
    vertexPtr[1] = static_cast<float>(i) * 0.5f;
    vertexPtr += 2;
  }
  
  // Allocate index data (3 indices for one triangle)
  size_t indexCount = 3;
  size_t indexDataSize = indexCount * sizeof(uint32_t);
  desc.indexData = te::core::Alloc(indexDataSize, alignof(uint32_t));
  assert(desc.indexData != nullptr);
  desc.indexDataSize = indexDataSize;
  
  uint32_t* indexPtr = static_cast<uint32_t*>(desc.indexData);
  indexPtr[0] = 0;
  indexPtr[1] = 1;
  indexPtr[2] = 2;
  
  // Create submesh
  SubmeshDesc submesh;
  submesh.offset = 0;
  submesh.count = static_cast<uint32_t>(indexCount);
  submesh.materialSlotIndex = 0;
  desc.submeshes.push_back(submesh);
  
  // Test CreateMesh
  MeshHandle handle = CreateMesh(&desc);
  assert(handle != nullptr);
  
  // Test GetSubmeshCount
  uint32_t submeshCount = GetSubmeshCount(handle);
  assert(submeshCount == 1);
  
  // Test GetSubmesh
  SubmeshDesc const* retrievedSubmesh = GetSubmesh(handle, 0);
  assert(retrievedSubmesh != nullptr);
  assert(retrievedSubmesh->offset == 0);
  assert(retrievedSubmesh->count == indexCount);
  assert(retrievedSubmesh->materialSlotIndex == 0);
  
  // Test GetSubmesh with invalid index
  assert(GetSubmesh(handle, 999) == nullptr);
  
  // Test GetLODCount (should be 0 for this mesh)
  uint32_t lodCount = GetLODCount(handle);
  assert(lodCount == 0);
  
  // Test SelectLOD (should return 0 for base LOD)
  uint32_t selectedLOD = SelectLOD(handle, 0.0f);
  assert(selectedLOD == 0);
  
  // Test GetSkinningData (should be nullptr for this mesh)
  SkinningData const* skinningData = GetSkinningData(handle);
  assert(skinningData == nullptr);
  
  // Test ReleaseMesh
  ReleaseMesh(handle);
  
  // Cleanup
  te::core::Free(desc.vertexData);
  te::core::Free(desc.indexData);
  
  return 0;
}
