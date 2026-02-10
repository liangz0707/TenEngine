/**
 * @file MeshDeserializer.cpp
 * @brief Mesh deserializer implementation.
 */
#include <te/mesh/MeshDeserializer.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/object/Serializer.h>
#include <te/object/TypeRegistry.h>
#include <te/core/alloc.h>
#include <memory>

namespace te {
namespace mesh {

void* MeshDeserializer::Deserialize(void const* buffer, size_t size) {
  if (!buffer || size == 0) {
    return nullptr;
  }
  
  // Get type descriptor for MeshAssetDesc
  te::object::TypeDescriptor const* typeDesc = te::object::TypeRegistry::GetTypeByName("MeshAssetDesc");
  if (!typeDesc) {
    return nullptr;
  }
  
  // Create MeshAssetDesc instance
  std::unique_ptr<MeshAssetDesc> desc = std::make_unique<MeshAssetDesc>();
  if (!desc) {
    return nullptr;
  }
  
  // Create deserializer
  std::unique_ptr<te::object::ISerializer> deserializer(te::object::CreateBinarySerializer());
  if (!deserializer) {
    return nullptr;
  }
  
  // Create SerializedBuffer from input buffer
  te::object::SerializedBuffer serializedBuffer{};
  serializedBuffer.data = const_cast<void*>(buffer);  // Non-const for interface, but we won't modify
  serializedBuffer.size = size;
  serializedBuffer.capacity = size;
  
  // Deserialize
  if (!deserializer->Deserialize(serializedBuffer, desc.get(), typeDesc->id)) {
    return nullptr;
  }
  
  // Return payload (caller takes ownership)
  return desc.release();
}

}  // namespace mesh
}  // namespace te
