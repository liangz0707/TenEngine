/**
 * @file Mesh.h
 * @brief Mesh handle and related types (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_H
#define TE_MESH_MESH_H

#include <te/rendercore/resource_desc.hpp>
#include <te/core/math.h>
#include <cstdint>
#include <cstddef>

namespace te {
namespace mesh {

// Forward declaration
namespace detail {
struct MeshData;
}

/**
 * Mesh handle (opaque handle).
 * Returned by CreateMesh, released by ReleaseMesh.
 */
using MeshHandle = detail::MeshData*;

/**
 * Submesh description.
 * Defines a draw call batch with material slot index.
 */
struct SubmeshDesc {
  uint32_t offset = 0;           // Index offset
  uint32_t count = 0;            // Index count
  uint32_t materialSlotIndex = 0; // Material slot index for this submesh
};

/**
 * LOD level description.
 * Defines a LOD level with distance/screen size thresholds.
 */
struct LODLevel {
  float distanceThreshold = 0.0f;    // Distance threshold for LOD selection
  float screenSizeThreshold = 0.0f;  // Screen size threshold (optional)
  uint32_t submeshStartIndex = 0;     // Starting submesh index for this LOD
  uint32_t submeshCount = 0;          // Number of submeshes in this LOD
};

/**
 * LOD handle (same as LODLevel index).
 */
using LODHandle = uint32_t;

/**
 * Skinning data.
 * Contains bone indices, weights, and bind pose matrices.
 */
struct SkinningData {
  // Bone indices per vertex (4 bones per vertex, -1 for unused slots)
  int32_t* boneIndices = nullptr;
  size_t boneIndicesCount = 0;  // Number of bone index entries (vertexCount * 4)
  
  // Bone weights per vertex (4 weights per vertex, normalized)
  float* boneWeights = nullptr;
  size_t boneWeightsCount = 0;  // Number of weight entries (vertexCount * 4)
  
  // Bind pose matrices (inverse bind pose)
  float* bindPoseMatrices = nullptr;
  size_t bindPoseMatrixCount = 0;  // Number of matrices (boneCount * 16)
  
  uint32_t boneCount = 0;  // Number of bones
  uint32_t vertexCount = 0; // Number of vertices with skinning data
};

/**
 * Get submesh count.
 * @param h Mesh handle
 * @return Number of submeshes
 */
uint32_t GetSubmeshCount(MeshHandle h);

/**
 * Get submesh description.
 * @param h Mesh handle
 * @param index Submesh index
 * @return Pointer to SubmeshDesc, or nullptr if index is invalid
 */
SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index);

/**
 * Get LOD count.
 * @param h Mesh handle
 * @return Number of LOD levels
 */
uint32_t GetLODCount(MeshHandle h);

/**
 * Select LOD level based on distance or screen size.
 * @param h Mesh handle
 * @param distanceOrScreenSize Distance or screen size value
 * @return Selected LOD level index
 */
uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize);

/**
 * Get LOD level description (submesh range for the given LOD).
 * @param h Mesh handle
 * @param lodIndex LOD level index (0..GetLODCount-1)
 * @param out Output LOD level; unchanged if invalid
 * @return true if lodIndex is valid
 */
bool GetLODLevel(MeshHandle h, uint32_t lodIndex, LODLevel* out);

/**
 * Get skinning data.
 * @param h Mesh handle
 * @return Pointer to SkinningData, or nullptr if mesh has no skinning data
 */
SkinningData const* GetSkinningData(MeshHandle h);

/**
 * Request streaming for LOD levels.
 * Optional: interface with 013-Resource streaming system.
 * @param h Mesh handle
 * @param lodLevel LOD level to request streaming for
 */
void RequestStreaming(MeshHandle h, uint32_t lodLevel);

/**
 * Get mesh local-space AABB (for frustum culling when node has no AABB).
 * @param h Mesh handle
 * @return Local AABB; min==max==0 if not set
 */
te::core::AABB GetMeshAABB(MeshHandle h);

/**
 * Get submesh local-space AABB if available; otherwise returns mesh AABB.
 * @param h Mesh handle
 * @param submeshIndex Submesh index
 * @return Local AABB for submesh or full mesh
 */
te::core::AABB GetSubmeshAABB(MeshHandle h, uint32_t submeshIndex);

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_H
