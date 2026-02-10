/**
 * @file CgltfImporter.cpp
 * @brief cgltf mesh importer implementation.
 */
#include <te/mesh/MeshAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cstring>

#ifdef TENENGINE_USE_CGLTF
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#endif

namespace te {
namespace mesh {

#ifdef TENENGINE_USE_CGLTF
/**
 * Import mesh from glTF file using cgltf.
 * @param sourcePath Source file path
 * @param outDesc Output MeshAssetDesc (caller takes ownership)
 * @return true on success, false on failure
 */
bool ImportMeshFromCgltf(char const* sourcePath, MeshAssetDesc* outDesc) {
  if (!sourcePath || !outDesc) {
    return false;
  }
  
  cgltf_options options = {};
  cgltf_data* data = nullptr;
  cgltf_result result = cgltf_parse_file(&options, sourcePath, &data);
  if (result != cgltf_result_success || !data) {
    return false;
  }
  
  result = cgltf_load_buffers(&options, data, sourcePath);
  if (result != cgltf_result_success) {
    cgltf_free(data);
    return false;
  }
  
  // Initialize MeshAssetDesc
  outDesc->formatVersion = 1;
  outDesc->debugDescription = "Imported from cgltf";
  
  // For now, import first mesh/primitive only
  // In a full implementation, we would handle multiple meshes/primitives
  if (data->meshes_count == 0 || data->meshes[0].primitives_count == 0) {
    cgltf_free(data);
    return false;
  }
  
  cgltf_mesh const& mesh = data->meshes[0];
  cgltf_primitive const& primitive = mesh.primitives[0];
  
  // Create vertex format
  rendercore::VertexAttribute attrs[8];  // Max attributes
  uint32_t attrCount = 0;
  uint32_t offset = 0;
  
  for (size_t i = 0; i < primitive.attributes_count && attrCount < 8; ++i) {
    cgltf_attribute const& attr = primitive.attributes[i];
    if (attr.type == cgltf_attribute_type_position) {
      attrs[attrCount].location = 0;
      attrs[attrCount].format = rendercore::VertexAttributeFormat::Float3;
      attrs[attrCount].offset = offset;
      offset += sizeof(float) * 3;
      ++attrCount;
    } else if (attr.type == cgltf_attribute_type_normal) {
      attrs[attrCount].location = 1;
      attrs[attrCount].format = rendercore::VertexAttributeFormat::Float3;
      attrs[attrCount].offset = offset;
      offset += sizeof(float) * 3;
      ++attrCount;
    } else if (attr.type == cgltf_attribute_type_texcoord) {
      attrs[attrCount].location = 2;
      attrs[attrCount].format = rendercore::VertexAttributeFormat::Float2;
      attrs[attrCount].offset = offset;
      offset += sizeof(float) * 2;
      ++attrCount;
    }
  }
  
  rendercore::VertexFormatDesc vertexFormatDesc{};
  vertexFormatDesc.attributes = attrs;
  vertexFormatDesc.attributeCount = attrCount;
  vertexFormatDesc.stride = offset;
  outDesc->vertexLayout = rendercore::CreateVertexFormat(vertexFormatDesc);
  
  // Get vertex data
  cgltf_accessor const* positionAccessor = nullptr;
  for (size_t i = 0; i < primitive.attributes_count; ++i) {
    if (primitive.attributes[i].type == cgltf_attribute_type_position) {
      positionAccessor = primitive.attributes[i].data;
      break;
    }
  }
  
  if (!positionAccessor) {
    cgltf_free(data);
    return false;
  }
  
  size_t vertexCount = positionAccessor->count;
  size_t vertexDataSize = vertexCount * vertexFormatDesc.stride;
  outDesc->vertexData = te::core::Alloc(vertexDataSize, alignof(float));
  if (!outDesc->vertexData) {
    cgltf_free(data);
    return false;
  }
  outDesc->vertexDataSize = vertexDataSize;
  
  // Copy vertex data (simplified - in full implementation would handle all attributes)
  uint8_t* vertexPtr = static_cast<uint8_t*>(outDesc->vertexData);
  cgltf_buffer_view const* positionView = positionAccessor->buffer_view;
  if (positionView && positionView->buffer->data) {
    uint8_t const* srcData = static_cast<uint8_t const*>(positionView->buffer->data);
    srcData += positionView->offset + positionAccessor->offset;
    size_t positionSize = vertexCount * sizeof(float) * 3;
    std::memcpy(vertexPtr, srcData, positionSize);
  }
  
  // Get index data
  cgltf_accessor const* indexAccessor = primitive.indices;
  if (!indexAccessor) {
    cgltf_free(data);
    te::core::Free(outDesc->vertexData);
    return false;
  }
  
  size_t indexCount = indexAccessor->count;
  outDesc->indexFormat.type = (indexAccessor->component_type == cgltf_component_type_r_16u) ?
    rendercore::IndexType::UInt16 : rendercore::IndexType::UInt32;
  size_t indexDataSize = indexCount * (outDesc->indexFormat.type == rendercore::IndexType::UInt16 ? sizeof(uint16_t) : sizeof(uint32_t));
  outDesc->indexData = te::core::Alloc(indexDataSize, alignof(uint32_t));
  if (!outDesc->indexData) {
    te::core::Free(outDesc->vertexData);
    cgltf_free(data);
    return false;
  }
  outDesc->indexDataSize = indexDataSize;
  
  // Copy index data
  cgltf_buffer_view const* indexView = indexAccessor->buffer_view;
  if (indexView && indexView->buffer->data) {
    uint8_t const* srcData = static_cast<uint8_t const*>(indexView->buffer->data);
    srcData += indexView->offset + indexAccessor->offset;
    std::memcpy(outDesc->indexData, srcData, indexDataSize);
  }
  
  // Create submesh
  SubmeshDesc submesh;
  submesh.offset = 0;
  submesh.count = static_cast<uint32_t>(indexCount);
  submesh.materialSlotIndex = primitive.material ? static_cast<uint32_t>(primitive.material - data->materials) : 0;
  outDesc->submeshes.push_back(submesh);
  
  cgltf_free(data);
  return true;
#else
  (void)sourcePath;
  (void)outDesc;
  return false;  // cgltf not enabled
#endif
}

}  // namespace mesh
}  // namespace te
