/**
 * @file MeshAssetDesc.cpp
 * @brief MeshAssetDesc type registration with 002-Object.
 */
#include <te/mesh/MeshAssetDesc.h>
#include <te/object/TypeRegistry.h>
#include <te/object/TypeId.h>
#include <te/core/alloc.h>
#include <cstring>

namespace te {
namespace mesh {

namespace {
  // Type ID for MeshAssetDesc (generated at registration time)
  static object::TypeId s_meshAssetDescTypeId = object::kInvalidTypeId;
  
  // Create instance function for MeshAssetDesc
  void* CreateMeshAssetDescInstance() {
    return te::core::Alloc(sizeof(MeshAssetDesc), alignof(MeshAssetDesc));
  }
  
  // Register MeshAssetDesc type with 002-Object TypeRegistry
  struct MeshAssetDescRegistrar {
    MeshAssetDescRegistrar() {
      // Generate type ID (simplified - in production use proper ID generation)
      static object::TypeId s_nextTypeId = 2000;  // Start from 2000 to avoid conflicts
      s_meshAssetDescTypeId = s_nextTypeId++;
      
      // Create property descriptors for MeshAssetDesc
      // Note: This is a simplified implementation. In a real implementation,
      // we would need to handle all fields including vectors and pointers.
      // For now, we'll register basic fields only.
      
      object::PropertyDescriptor props[] = {
        {"formatVersion", 0, offsetof(MeshAssetDesc, formatVersion), sizeof(uint32_t), nullptr},
        {"debugDescription", 0, offsetof(MeshAssetDesc, debugDescription), sizeof(std::string), nullptr},
        // Note: vertexLayout, vertexData, indexData, etc. are complex types
        // that require custom serialization. We'll handle them in the serializer.
      };
      
      object::TypeDescriptor desc;
      desc.id = s_meshAssetDescTypeId;
      desc.name = "MeshAssetDesc";
      desc.size = sizeof(MeshAssetDesc);
      desc.properties = props;
      desc.propertyCount = 2;  // Only basic fields for now
      desc.baseTypeId = object::kInvalidTypeId;
      desc.createInstance = CreateMeshAssetDescInstance;
      
      object::TypeRegistry::RegisterType(desc);
    }
  };
  
  // Static registrar instance (registers type on module load)
  static MeshAssetDescRegistrar g_registrar;
}

}  // namespace mesh
}  // namespace te
