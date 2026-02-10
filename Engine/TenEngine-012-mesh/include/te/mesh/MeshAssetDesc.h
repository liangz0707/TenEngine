/**
 * @file MeshAssetDesc.h
 * @brief Mesh asset description (contract: specs/_contracts/012-mesh-ABI.md).
 * 
 * MeshAssetDesc belongs to 012-Mesh module.
 * Serialized format: .mesh (AssetDesc) + .meshdata (binary data).
 */
#ifndef TE_MESH_MESH_ASSET_DESC_H
#define TE_MESH_MESH_ASSET_DESC_H

#include <te/rendercore/resource_desc.hpp>
#include <te/mesh/Mesh.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace te {
namespace mesh {

/**
 * Mesh asset description.
 * Contains all data needed to create a mesh resource.
 * 
 * Note: vertexData and indexData are pointers to data that will be stored
 * in the .meshdata file. During serialization, these pointers are not serialized
 * directly; instead, the data is written to the .meshdata file and the pointers
 * are set to nullptr in the serialized AssetDesc.
 */
struct MeshAssetDesc {
  uint32_t formatVersion = 1;  // Format version for compatibility checking
  std::string debugDescription; // Debug description (optional)
  
  // Vertex format and data
  rendercore::VertexFormat vertexLayout;
  void* vertexData = nullptr;      // Pointer to vertex data (not serialized directly)
  size_t vertexDataSize = 0;       // Size of vertex data in bytes
  
  // Index format and data
  void* indexData = nullptr;        // Pointer to index data (not serialized directly)
  size_t indexDataSize = 0;         // Size of index data in bytes
  rendercore::IndexFormat indexFormat;
  
  // Submeshes
  std::vector<SubmeshDesc> submeshes;
  
  // Optional LOD levels
  std::vector<LODLevel> lodLevels;
  
  // Optional skinning data
  SkinningData* skinningData = nullptr;  // nullptr if no skinning data
  
  /**
   * Check if description is valid.
   * @return true if valid, false otherwise
   */
  bool IsValid() const {
    return vertexLayout.IsValid() && 
           indexFormat.IsValid() && 
           vertexDataSize > 0 && 
           indexDataSize > 0 &&
           !submeshes.empty();
  }
};

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_ASSET_DESC_H
