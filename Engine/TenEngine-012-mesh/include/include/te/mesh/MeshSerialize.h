/** 012-Mesh: SerializeMeshToBuffer；013 Save 时按类型调用。 */
#ifndef TE_MESH_MESH_SERIALIZE_H
#define TE_MESH_MESH_SERIALIZE_H

#include <cstddef>
#include "te/mesh/Mesh.h"

namespace te {
namespace mesh {

/** 从 handle 产出 .mesh 布局内存；*size 为所需/已写字节数。 */
bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size);

}  // namespace mesh
}  // namespace te

#endif
