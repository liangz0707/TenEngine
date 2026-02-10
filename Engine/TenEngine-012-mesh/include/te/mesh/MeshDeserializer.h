/**
 * @file MeshDeserializer.h
 * @brief Mesh deserializer (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_DESERIALIZER_H
#define TE_MESH_MESH_DESERIALIZER_H

#include <cstddef>

namespace te {
namespace mesh {

/**
 * MeshDeserializer: deserializes mesh data from buffer.
 * Implements IDeserializer interface.
 */
class MeshDeserializer {
 public:
  /**
   * Deserialize mesh data from buffer.
   * Calls 002-Object deserialization interface.
   * 
   * @param buffer Buffer containing serialized mesh data
   * @param size Buffer size in bytes
   * @return MeshAssetDesc* (payload), or nullptr on failure
   */
  static void* Deserialize(void const* buffer, size_t size);
};

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_DESERIALIZER_H
