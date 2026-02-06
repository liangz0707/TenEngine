/** 012-mesh 内部表示；仅 src/mesh 使用。 */
#ifndef TE_MESH_DETAIL_MESH_DATA_HPP
#define TE_MESH_DETAIL_MESH_DATA_HPP

#include "te/mesh/Mesh.h"

namespace te {
namespace mesh {
namespace detail {

struct MeshData {
  uint32_t submeshCount = 0;
  SubmeshDesc* submeshes = nullptr;  // owned
  uint32_t lodCount = 0;
  SkinningData const* skinningData = nullptr;  // non-owned
};

inline MeshData* get(MeshHandle h) {
  return static_cast<MeshData*>(h.opaque);
}

}  // namespace detail
}  // namespace mesh
}  // namespace te

#endif
