/**
 * @file MeshSerialize.h
 * @brief Mesh serialization (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_SERIALIZE_H
#define TE_MESH_MESH_SERIALIZE_H

#include <te/mesh/Mesh.h>
#include <cstddef>

namespace te {
namespace mesh {

/**
 * Serialize mesh to buffer.
 * Called by ResourceManager when saving Mesh resource.
 * 
 * @param h Mesh handle
 * @param buffer Output buffer (allocated by caller, must be large enough)
 * @param size Input: buffer size, Output: actual serialized size
 * @return true on success, false on failure
 */
bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size);

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_SERIALIZE_H
