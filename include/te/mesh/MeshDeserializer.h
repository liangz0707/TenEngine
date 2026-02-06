/** 012-Mesh: MeshDeserializer (IDeserializer); Deserialize .mesh -> MeshAssetDesc*. */
#ifndef TE_MESH_MESH_DESERIALIZER_H
#define TE_MESH_MESH_DESERIALIZER_H

#include <cstddef>

namespace te {
namespace mesh {

class MeshDeserializer {
 public:
  /** 产出 MeshAssetDesc*（opaque payload），013 不解析。 */
  void* Deserialize(void const* buffer, size_t size);
};

}  // namespace mesh
}  // namespace te

#endif
