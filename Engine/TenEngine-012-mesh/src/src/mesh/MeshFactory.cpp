// 012-mesh: CreateMesh, ReleaseMesh (T007, T008)
#include "te/mesh/MeshFactory.h"
#include "te/mesh/MeshAssetDesc.h"
#include "te/mesh/detail/mesh_data.hpp"
#include "te/core/alloc.h"
#include <cstring>

namespace te {
namespace mesh {

using detail::MeshData;

MeshHandle CreateMesh(MeshAssetDesc const* desc) {
  if (!desc) return MeshHandle{};
  MeshData* d = static_cast<MeshData*>(te::core::Alloc(sizeof(MeshData), alignof(MeshData)));
  if (!d) return MeshHandle{};
  d->submeshCount = desc->submeshCount;
  d->submeshes = nullptr;
  d->lodCount = desc->lodCount;
  d->skinningData = desc->skinningData;
  if (desc->submeshCount > 0 && desc->submeshes) {
    d->submeshes = static_cast<SubmeshDesc*>(
        te::core::Alloc(desc->submeshCount * sizeof(SubmeshDesc), alignof(SubmeshDesc)));
    if (d->submeshes)
      std::memcpy(d->submeshes, desc->submeshes, desc->submeshCount * sizeof(SubmeshDesc));
  }
  MeshHandle h;
  h.opaque = d;
  return h;
}

void ReleaseMesh(MeshHandle h) {
  if (!h.opaque) return;
  MeshData* d = static_cast<MeshData*>(h.opaque);
  if (d->submeshes) te::core::Free(d->submeshes);
  te::core::Free(d);
}

}  // namespace mesh
}  // namespace te
