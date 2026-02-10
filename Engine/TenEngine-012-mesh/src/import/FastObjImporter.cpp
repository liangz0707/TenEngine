/**
 * @file FastObjImporter.cpp
 * @brief fast_obj mesh importer implementation.
 */
#include <te/mesh/MeshAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cstring>

#ifdef TENENGINE_USE_FAST_OBJ
#define FAST_OBJ_IMPLEMENTATION
#include "fast_obj.h"
#endif

namespace te {
namespace mesh {

#ifdef TENENGINE_USE_FAST_OBJ
/**
 * Import mesh from OBJ file using fast_obj.
 * @param sourcePath Source file path
 * @param outDesc Output MeshAssetDesc (caller takes ownership)
 * @return true on success, false on failure
 */
bool ImportMeshFromFastObj(char const* sourcePath, MeshAssetDesc* outDesc) {
  if (!sourcePath || !outDesc) {
    return false;
  }
  
  fastObjMesh* mesh = fast_obj_read(sourcePath);
  if (!mesh) {
    return false;
  }
  
  // Initialize MeshAssetDesc
  outDesc->formatVersion = 1;
  outDesc->debugDescription = "Imported from fast_obj";
  
  // Create vertex format (position + normal + UV)
  rendercore::VertexAttribute attrs[3];
  uint32_t offset = 0;
  
  attrs[0].location = 0;
  attrs[0].format = rendercore::VertexAttributeFormat::Float3;
  attrs[0].offset = offset;
  offset += sizeof(float) * 3;
  
  attrs[1].location = 1;
  attrs[1].format = rendercore::VertexAttributeFormat::Float3;
  attrs[1].offset = offset;
  offset += sizeof(float) * 3;
  
  attrs[2].location = 2;
  attrs[2].format = rendercore::VertexAttributeFormat::Float2;
  attrs[2].offset = offset;
  offset += sizeof(float) * 2;
  
  rendercore::VertexFormatDesc vertexFormatDesc{};
  vertexFormatDesc.attributes = attrs;
  vertexFormatDesc.attributeCount = 3;
  vertexFormatDesc.stride = offset;
  outDesc->vertexLayout = rendercore::CreateVertexFormat(vertexFormatDesc);
  
  // Count vertices and indices
  // fast_obj uses 1-based indexing for positions/texcoords/normals
  // Find max vertex index to determine vertex count
  size_t maxVertexIndex = 0;
  for (unsigned int i = 0; i < mesh->index_count; ++i) {
    fastObjIndex const& idx = mesh->indices[i];
    if (idx.p > maxVertexIndex) {
      maxVertexIndex = idx.p;
    }
  }
  size_t vertexCount = maxVertexIndex;  // Indices are 1-based, so max index = count
  
  // Count total indices after triangulation
  size_t indexCount = 0;
  for (unsigned int i = 0; i < mesh->face_count; ++i) {
    unsigned int faceVertCount = mesh->face_vertices[i];
    if (faceVertCount >= 3) {
      indexCount += (faceVertCount - 2) * 3;  // Triangulate: (n-2) triangles per n-gon
    }
  }
  
  // Allocate and copy vertex data
  size_t vertexDataSize = vertexCount * vertexFormatDesc.stride;
  outDesc->vertexData = te::core::Alloc(vertexDataSize, alignof(float));
  if (!outDesc->vertexData) {
    fast_obj_destroy(mesh);
    return false;
  }
  outDesc->vertexDataSize = vertexDataSize;
  
  // Copy vertex data (fast_obj arrays are 1-based, index 0 is dummy)
  uint8_t* vertexPtr = static_cast<uint8_t*>(outDesc->vertexData);
  for (size_t i = 0; i < vertexCount; ++i) {
    size_t srcIdx = i + 1;  // Convert to 1-based for fast_obj arrays
    
    // Position
    if (srcIdx < mesh->position_count && mesh->positions) {
      std::memcpy(vertexPtr, &mesh->positions[srcIdx * 3], sizeof(float) * 3);
    } else {
      // Zero fill if missing
      std::memset(vertexPtr, 0, sizeof(float) * 3);
    }
    vertexPtr += sizeof(float) * 3;
    
    // Normal
    if (srcIdx < mesh->normal_count && mesh->normals) {
      std::memcpy(vertexPtr, &mesh->normals[srcIdx * 3], sizeof(float) * 3);
    } else {
      // Default normal (0, 1, 0)
      float defaultNormal[3] = {0.0f, 1.0f, 0.0f};
      std::memcpy(vertexPtr, defaultNormal, sizeof(float) * 3);
    }
    vertexPtr += sizeof(float) * 3;
    
    // UV
    if (srcIdx < mesh->texcoord_count && mesh->texcoords) {
      std::memcpy(vertexPtr, &mesh->texcoords[srcIdx * 2], sizeof(float) * 2);
    } else {
      // Zero fill if missing
      std::memset(vertexPtr, 0, sizeof(float) * 2);
    }
    vertexPtr += sizeof(float) * 2;
  }
  
  // Allocate and copy index data
  outDesc->indexFormat.type = rendercore::IndexType::UInt32;
  size_t indexDataSize = indexCount * sizeof(uint32_t);
  outDesc->indexData = te::core::Alloc(indexDataSize, alignof(uint32_t));
  if (!outDesc->indexData) {
    te::core::Free(outDesc->vertexData);
    fast_obj_destroy(mesh);
    return false;
  }
  outDesc->indexDataSize = indexDataSize;
  
  // Convert fast_obj indices to flat indices and triangulate faces
  uint32_t* indexPtr = static_cast<uint32_t*>(outDesc->indexData);
  uint32_t globalIndexOffset = 0;
  
  // Process each group
  for (unsigned int i = 0; i < mesh->group_count; ++i) {
    fastObjGroup const& group = mesh->groups[i];
    
    // Calculate number of indices for this group
    unsigned int groupIndexCount = 0;
    for (unsigned int f = 0; f < group.face_count; ++f) {
      unsigned int faceIdx = group.face_offset + f;
      if (faceIdx < mesh->face_count) {
        unsigned int faceVertCount = mesh->face_vertices[faceIdx];
        if (faceVertCount >= 3) {
          groupIndexCount += (faceVertCount - 2) * 3;  // Triangulate
        }
      }
    }
    
    SubmeshDesc submesh;
    submesh.offset = globalIndexOffset;
    submesh.count = groupIndexCount;
    // Get material from first face in group
    unsigned int materialSlot = 0;
    if (group.face_offset < mesh->face_count && mesh->face_materials) {
      materialSlot = mesh->face_materials[group.face_offset];
    }
    submesh.materialSlotIndex = materialSlot;
    outDesc->submeshes.push_back(submesh);
    
    // Process faces in this group and triangulate
    unsigned int groupIndexOffset = group.index_offset;
    for (unsigned int f = 0; f < group.face_count; ++f) {
      unsigned int faceIdx = group.face_offset + f;
      if (faceIdx >= mesh->face_count) {
        break;
      }
      
      unsigned int faceVertCount = mesh->face_vertices[faceIdx];
      if (faceVertCount < 3) {
        continue;  // Skip invalid faces
      }
      
      // Triangulate: for n vertices, create (n-2) triangles
      // Triangle fan: vertex 0, vertex i+1, vertex i+2 for i in [0, n-3]
      for (unsigned int t = 0; t < faceVertCount - 2; ++t) {
        unsigned int idx0 = groupIndexOffset;
        unsigned int idx1 = groupIndexOffset + t + 1;
        unsigned int idx2 = groupIndexOffset + t + 2;
        
        if (idx0 < mesh->index_count && idx1 < mesh->index_count && idx2 < mesh->index_count) {
          // Convert from 1-based to 0-based indexing
          indexPtr[globalIndexOffset++] = mesh->indices[idx0].p - 1;
          indexPtr[globalIndexOffset++] = mesh->indices[idx1].p - 1;
          indexPtr[globalIndexOffset++] = mesh->indices[idx2].p - 1;
        }
      }
      
      groupIndexOffset += faceVertCount;
    }
  }
  
  fast_obj_destroy(mesh);
  return true;
#else
  (void)sourcePath;
  (void)outDesc;
  return false;  // fast_obj not enabled
#endif
}

}  // namespace mesh
}  // namespace te
