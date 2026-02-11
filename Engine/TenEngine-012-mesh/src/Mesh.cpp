/**
 * @file Mesh.cpp
 * @brief Mesh handle implementation.
 */
#include <te/mesh/Mesh.h>
#include <te/mesh/detail/mesh_data.hpp>
#include <cassert>

namespace te {
namespace mesh {

uint32_t GetSubmeshCount(MeshHandle h) {
  if (!h) {
    return 0;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  return static_cast<uint32_t>(data->submeshes.size());
}

SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index) {
  if (!h) {
    return nullptr;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  if (index >= data->submeshes.size()) {
    return nullptr;
  }
  return &data->submeshes[index];
}

uint32_t GetLODCount(MeshHandle h) {
  if (!h) {
    return 0;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  return static_cast<uint32_t>(data->lodLevels.size());
}

uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize) {
  if (!h) {
    return 0;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  if (data->lodLevels.empty()) {
    return 0;  // Return base LOD (index 0)
  }
  
  // Select LOD based on distance/screen size threshold
  // Lower index = higher quality (closer/smaller threshold)
  for (size_t i = 0; i < data->lodLevels.size(); ++i) {
    if (distanceOrScreenSize >= data->lodLevels[i].distanceThreshold) {
      return static_cast<uint32_t>(i);
    }
  }
  
  // Return highest LOD if all thresholds exceeded
  return static_cast<uint32_t>(data->lodLevels.size() - 1);
}

bool GetLODLevel(MeshHandle h, uint32_t lodIndex, LODLevel* out) {
  if (!h || !out) return false;
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  if (lodIndex >= data->lodLevels.size()) return false;
  *out = data->lodLevels[lodIndex];
  return true;
}

SkinningData const* GetSkinningData(MeshHandle h) {
  if (!h) {
    return nullptr;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  if (!data->skinningData) {
    return nullptr;
  }
  return data->skinningData.get();
}

void RequestStreaming(MeshHandle h, uint32_t lodLevel) {
  // Optional: interface with 013-Resource streaming system
  // For now, this is a placeholder
  (void)h;
  (void)lodLevel;
}

te::core::AABB GetMeshAABB(MeshHandle h) {
  if (!h) return te::core::AABB{};
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  return data->localAABB;
}

te::core::AABB GetSubmeshAABB(MeshHandle h, uint32_t submeshIndex) {
  te::core::AABB meshAabb = GetMeshAABB(h);
  (void)submeshIndex;
  /* Per-submesh AABB could be stored in MeshData later; for now return mesh AABB */
  return meshAabb;
}

uint32_t GetVertexStride(MeshHandle h) {
  if (!h) return 32u;
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  if (data->vertexLayout.IsValid() && data->vertexLayout.stride > 0u)
    return data->vertexLayout.stride;
  return 32u;
}

uint32_t GetIndexFormat(MeshHandle h) {
  if (!h) return 0u;
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  return (data->indexFormat.type == te::rendercore::IndexType::UInt32) ? 1u : 0u;
}

}  // namespace mesh
}  // namespace te
