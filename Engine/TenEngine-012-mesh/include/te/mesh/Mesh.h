/** 012-Mesh: MeshHandle, SubmeshDesc, LODLevel, SkinningData; query APIs. */
#ifndef TE_MESH_MESH_H
#define TE_MESH_MESH_H

#include <cstdint>

namespace te {
namespace mesh {

/** 不透明网格句柄；CreateMesh 返回，ReleaseMesh 释放。 */
struct MeshHandle {
  void* opaque = nullptr;
  bool valid() const { return opaque != nullptr; }
};

/** 子网格描述：offset, count, materialSlotIndex；DrawCall 批次。 */
struct SubmeshDesc {
  uint32_t offset = 0;
  uint32_t count = 0;
  uint32_t materialSlotIndex = 0;
};

/** LOD 级别/句柄；与 Resource 流式对接。 */
struct LODLevel {
  uint32_t index = 0;
  float distanceOrScreenSize = 0.f;
};

/** 蒙皮数据：BoneIndices, Weights, BindPose；与 015-Animation 对接。 */
struct SkinningData {
  void const* boneIndices = nullptr;
  void const* weights = nullptr;
  void const* bindPose = nullptr;
  uint32_t boneCount = 0;
};

uint32_t GetSubmeshCount(MeshHandle h);
SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index);
uint32_t GetLODCount(MeshHandle h);
uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize);
SkinningData const* GetSkinningData(MeshHandle h);

}  // namespace mesh
}  // namespace te

#endif
