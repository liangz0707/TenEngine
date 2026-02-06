/** 012-Mesh: MeshAssetDesc（归属 012）；.mesh 序列化格式。 */
#ifndef TE_MESH_MESH_ASSET_DESC_H
#define TE_MESH_MESH_ASSET_DESC_H

#include <cstdint>
#include "te/mesh/Mesh.h"

namespace te {
namespace mesh {

/** 网格描述：formatVersion, vertexLayout, vertexData, indexData, submeshes, 可选 LOD/蒙皮。 */
struct MeshAssetDesc {
  uint32_t formatVersion = 0;
  char const* debugDescription = nullptr;
  void const* vertexLayout = nullptr;   /* 与 009 VertexFormat 一致 */
  void const* vertexData = nullptr;
  uint64_t vertexDataSize = 0;
  void const* indexFormat = nullptr;    /* 与 009 IndexFormat 一致 */
  void const* indexData = nullptr;
  uint64_t indexDataSize = 0;
  SubmeshDesc const* submeshes = nullptr;
  uint32_t submeshCount = 0;
  /* 可选 LOD：由实现扩展 */
  void const* lodData = nullptr;
  uint32_t lodCount = 0;
  /* 可选蒙皮 */
  SkinningData const* skinningData = nullptr;
};

}  // namespace mesh
}  // namespace te

#endif
