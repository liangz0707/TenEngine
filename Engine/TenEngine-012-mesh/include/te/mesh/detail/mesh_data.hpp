/**
 * @file mesh_data.hpp
 * @brief Internal mesh data structure implementation details.
 */
#ifndef TE_MESH_DETAIL_MESH_DATA_HPP
#define TE_MESH_DETAIL_MESH_DATA_HPP

#include <te/mesh/Mesh.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/core/alloc.h>
#include <te/core/math.h>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <memory>

namespace te {
namespace mesh {
namespace detail {

/**
 * Internal mesh data structure.
 * Stores all mesh data: vertices, indices, submeshes, LOD (CPU-only; no device buffers).
 */
struct MeshData {
  // Custom deleter for te::core::Alloc memory
  struct CoreAllocDeleter {
    void operator()(uint8_t* ptr) const {
      if (ptr) {
        te::core::Free(ptr);
      }
    }
  };
  
  // Vertex and index data
  std::unique_ptr<uint8_t[], CoreAllocDeleter> vertexData;  // Owned vertex data
  size_t vertexDataSize = 0;
  std::unique_ptr<uint8_t[], CoreAllocDeleter> indexData;    // Owned index data
  size_t indexDataSize = 0;
  
  // Vertex and index formats
  rendercore::VertexFormat vertexLayout;
  rendercore::IndexFormat indexFormat;
  
  // Submeshes
  std::vector<SubmeshDesc> submeshes;
  
  // LOD levels
  std::vector<LODLevel> lodLevels;
  
  // Skinning data (optional)
  std::unique_ptr<SkinningData> skinningData;
  
  // Reference count (for resource management)
  uint32_t refCount = 1;

  /** Local-space AABB; used for frustum culling when node has no HasAABB. */
  te::core::AABB localAABB{};

  MeshData() = default;
  ~MeshData();
  
  // Non-copyable
  MeshData(MeshData const&) = delete;
  MeshData& operator=(MeshData const&) = delete;
  
  // Movable
  MeshData(MeshData&&) = default;
  MeshData& operator=(MeshData&&) = default;
};

}  // namespace detail
}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_DETAIL_MESH_DATA_HPP
