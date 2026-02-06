// 012-mesh: GetSubmeshCount, GetSubmesh, GetLODCount, SelectLOD, GetSkinningData (T009, T015, T020)
#include "te/mesh/Mesh.h"
#include "te/mesh/detail/mesh_data.hpp"

namespace te {
namespace mesh {

using detail::MeshData;
using detail::get;

uint32_t GetSubmeshCount(MeshHandle h) {
  MeshData* d = get(h);
  return d ? d->submeshCount : 0;
}

SubmeshDesc const* GetSubmesh(MeshHandle h, uint32_t index) {
  MeshData* d = get(h);
  if (!d || !d->submeshes || index >= d->submeshCount) return nullptr;
  return &d->submeshes[index];
}

uint32_t GetLODCount(MeshHandle h) {
  MeshData* d = get(h);
  return d ? d->lodCount : 0;
}

uint32_t SelectLOD(MeshHandle h, float distanceOrScreenSize) {
  (void)distanceOrScreenSize;
  MeshData* d = get(h);
  if (!d || d->lodCount == 0) return 0;
  if (d->lodCount <= 1) return 0;
  // Simple strategy: more distance -> higher LOD index (lower detail)
  if (distanceOrScreenSize <= 0.f) return 0;
  if (distanceOrScreenSize >= 1000.f) return d->lodCount - 1;
  uint32_t idx = static_cast<uint32_t>(distanceOrScreenSize / 100.f);
  if (idx >= d->lodCount) idx = d->lodCount - 1;
  return idx;
}

SkinningData const* GetSkinningData(MeshHandle h) {
  MeshData* d = get(h);
  return d ? d->skinningData : nullptr;
}

}  // namespace mesh
}  // namespace te
