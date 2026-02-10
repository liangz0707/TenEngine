/**
 * @file MeshSerialize.cpp
 * @brief Mesh serialization implementation.
 */
#include <te/mesh/MeshSerialize.h>
#include <te/mesh/detail/mesh_data.hpp>
#include <te/mesh/MeshAssetDesc.h>
#include <te/object/Serializer.h>
#include <te/object/TypeRegistry.h>
#include <te/core/alloc.h>
#include <memory>
#include <cstring>

namespace te {
namespace mesh {

bool SerializeMeshToBuffer(MeshHandle h, void* buffer, size_t* size) {
  if (!h || !buffer || !size || *size == 0) {
    return false;
  }
  
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  
  // Create MeshAssetDesc from mesh data
  MeshAssetDesc desc;
  desc.formatVersion = 1;
  desc.vertexLayout = data->vertexLayout;
  desc.indexFormat = data->indexFormat;
  // Note: vertexData and indexData pointers are not serialized directly
  // They are stored in the .meshdata file separately
  desc.vertexData = nullptr;  // Will be loaded from .meshdata file
  desc.vertexDataSize = data->vertexDataSize;
  desc.indexData = nullptr;  // Will be loaded from .meshdata file
  desc.indexDataSize = data->indexDataSize;
  desc.submeshes = data->submeshes;
  desc.lodLevels = data->lodLevels;
  // SkinningData pointer is also not serialized directly
  // If skinning data exists, it should be stored separately or serialized inline
  // For now, we set it to nullptr during serialization
  // The actual skinning data would need to be handled separately if needed
  desc.skinningData = nullptr;
  
  // Get type descriptor
  te::object::TypeDescriptor const* typeDesc = te::object::TypeRegistry::GetTypeByName("MeshAssetDesc");
  if (!typeDesc) {
    return false;
  }
  
  // Create serializer
  std::unique_ptr<te::object::ISerializer> serializer(te::object::CreateBinarySerializer());
  if (!serializer) {
    return false;
  }
  
  // Serialize
  te::object::SerializedBuffer serializedBuffer{};
  if (!serializer->Serialize(serializedBuffer, &desc, typeDesc->id)) {
    return false;
  }
  
  // Check buffer size
  if (serializedBuffer.size > *size) {
    *size = serializedBuffer.size;
    return false;  // Buffer too small
  }
  
  // Copy to output buffer
  std::memcpy(buffer, serializedBuffer.data, serializedBuffer.size);
  *size = serializedBuffer.size;
  
  // Free serialized buffer
  if (serializedBuffer.data) {
    te::core::Free(serializedBuffer.data);
  }
  
  return true;
}

}  // namespace mesh
}  // namespace te
