/**
 * @file AssimpImporter.cpp
 * @brief Assimp mesh importer implementation.
 */
#include <te/mesh/MeshAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cstring>
#include <string>

#ifdef TENENGINE_USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

namespace te {
namespace mesh {

#ifdef TENENGINE_USE_ASSIMP
/**
 * Import mesh from source file using Assimp.
 * @param sourcePath Source file path
 * @param outDesc Output MeshAssetDesc (caller takes ownership)
 * @return true on success, false on failure
 */
bool ImportMeshFromAssimp(char const* sourcePath, MeshAssetDesc* outDesc) {
  if (!sourcePath || !outDesc) {
    return false;
  }
  
  Assimp::Importer importer;
  
  // Import scene
  aiScene const* scene = importer.ReadFile(sourcePath,
    aiProcess_Triangulate |
    aiProcess_GenNormals |
    aiProcess_CalcTangentSpace |
    aiProcess_JoinIdenticalVertices);
  
  if (!scene || !scene->mRootNode || scene->mNumMeshes == 0) {
    return false;
  }
  
  // For now, import first mesh only
  // In a full implementation, we would handle multiple meshes
  aiMesh const* mesh = scene->mMeshes[0];
  if (!mesh) {
    return false;
  }
  
  // Initialize MeshAssetDesc
  outDesc->formatVersion = 1;
  outDesc->debugDescription = "Imported from Assimp";
  
  // Create vertex format (position + normal + UV)
  rendercore::VertexAttribute attrs[3];
  uint32_t offset = 0;
  
  // Position
  attrs[0].location = 0;
  attrs[0].format = rendercore::VertexAttributeFormat::Float3;
  attrs[0].offset = offset;
  offset += sizeof(float) * 3;
  
  // Normal
  if (mesh->HasNormals()) {
    attrs[1].location = 1;
    attrs[1].format = rendercore::VertexAttributeFormat::Float3;
    attrs[1].offset = offset;
    offset += sizeof(float) * 3;
  }
  
  // UV
  if (mesh->HasTextureCoords(0)) {
    attrs[2].location = 2;
    attrs[2].format = rendercore::VertexAttributeFormat::Float2;
    attrs[2].offset = offset;
    offset += sizeof(float) * 2;
  }
  
  rendercore::VertexFormatDesc vertexFormatDesc{};
  vertexFormatDesc.attributes = attrs;
  vertexFormatDesc.attributeCount = mesh->HasNormals() && mesh->HasTextureCoords(0) ? 3 : (mesh->HasNormals() ? 2 : 1);
  vertexFormatDesc.stride = offset;
  outDesc->vertexLayout = rendercore::CreateVertexFormat(vertexFormatDesc);
  
  // Allocate and copy vertex data
  size_t vertexCount = mesh->mNumVertices;
  size_t vertexDataSize = vertexCount * vertexFormatDesc.stride;
  outDesc->vertexData = te::core::Alloc(vertexDataSize, alignof(float));
  if (!outDesc->vertexData) {
    return false;
  }
  outDesc->vertexDataSize = vertexDataSize;
  
  uint8_t* vertexPtr = static_cast<uint8_t*>(outDesc->vertexData);
  for (size_t i = 0; i < vertexCount; ++i) {
    // Position
    std::memcpy(vertexPtr, &mesh->mVertices[i], sizeof(float) * 3);
    vertexPtr += sizeof(float) * 3;
    
    // Normal
    if (mesh->HasNormals()) {
      std::memcpy(vertexPtr, &mesh->mNormals[i], sizeof(float) * 3);
      vertexPtr += sizeof(float) * 3;
    }
    
    // UV
    if (mesh->HasTextureCoords(0)) {
      std::memcpy(vertexPtr, &mesh->mTextureCoords[0][i], sizeof(float) * 2);
      vertexPtr += sizeof(float) * 2;
    }
  }
  
  // Allocate and copy index data
  size_t indexCount = mesh->mNumFaces * 3;  // Triangulated
  outDesc->indexFormat.type = rendercore::IndexType::UInt32;
  size_t indexDataSize = indexCount * sizeof(uint32_t);
  outDesc->indexData = te::core::Alloc(indexDataSize, alignof(uint32_t));
  if (!outDesc->indexData) {
    te::core::Free(outDesc->vertexData);
    return false;
  }
  outDesc->indexDataSize = indexDataSize;
  
  uint32_t* indexPtr = static_cast<uint32_t*>(outDesc->indexData);
  for (size_t i = 0; i < mesh->mNumFaces; ++i) {
    aiFace const& face = mesh->mFaces[i];
    if (face.mNumIndices == 3) {
      indexPtr[0] = face.mIndices[0];
      indexPtr[1] = face.mIndices[1];
      indexPtr[2] = face.mIndices[2];
      indexPtr += 3;
    }
  }
  
  // Create submesh
  SubmeshDesc submesh;
  submesh.offset = 0;
  submesh.count = static_cast<uint32_t>(indexCount);
  submesh.materialSlotIndex = mesh->mMaterialIndex;
  outDesc->submeshes.push_back(submesh);
  
  return true;
#else
  (void)sourcePath;
  (void)outDesc;
  return false;  // Assimp not enabled
#endif
}

}  // namespace mesh
}  // namespace te
