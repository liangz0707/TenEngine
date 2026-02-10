/**
 * @file MeshModuleInit.cpp
 * @brief Mesh module initialization - register resource factory and deserializer.
 */
#include <te/mesh/MeshModuleInit.h>
#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/mesh/Mesh.h>
#include <te/resource/ResourceManager.h>
#include <te/resource/ResourceTypes.h>
#include <te/object/TypeRegistry.h>
#include <te/object/TypeId.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cstddef>

namespace te {
namespace mesh {

namespace {
  // Type ID for MeshAssetDesc (0x01200001 = module 012, type 1)
  constexpr object::TypeId kMeshAssetDescTypeId = 0x01200001u;

  // Resource factory function
  resource::IResource* CreateMeshResource(resource::ResourceType type) {
    if (type == resource::ResourceType::Mesh) {
      return new MeshResource();
    }
    return nullptr;
  }

  // Factory function to create MeshAssetDesc instance
  void* CreateMeshAssetDesc() {
    void* ptr = te::core::Alloc(sizeof(MeshAssetDesc), alignof(MeshAssetDesc));
    if (ptr) {
      new(ptr) MeshAssetDesc();
    }
    return ptr;
  }

  // Property descriptors for MeshAssetDesc
  // Note: std::string and std::vector properties use TypeId 0 (unknown type)
  // The serializer will handle these complex types specially
  object::PropertyDescriptor g_meshAssetDescProperties[] = {
    {"formatVersion", 0, offsetof(MeshAssetDesc, formatVersion), sizeof(uint32_t), nullptr},
    {"debugDescription", 0, offsetof(MeshAssetDesc, debugDescription), sizeof(std::string), nullptr},
    {"vertexLayout", 0, offsetof(MeshAssetDesc, vertexLayout), sizeof(rendercore::VertexFormat), nullptr},
    {"vertexData", 0, offsetof(MeshAssetDesc, vertexData), sizeof(void*), nullptr},
    {"vertexDataSize", 0, offsetof(MeshAssetDesc, vertexDataSize), sizeof(size_t), nullptr},
    {"indexData", 0, offsetof(MeshAssetDesc, indexData), sizeof(void*), nullptr},
    {"indexDataSize", 0, offsetof(MeshAssetDesc, indexDataSize), sizeof(size_t), nullptr},
    {"indexFormat", 0, offsetof(MeshAssetDesc, indexFormat), sizeof(rendercore::IndexFormat), nullptr},
    {"submeshes", 0, offsetof(MeshAssetDesc, submeshes), sizeof(std::vector<SubmeshDesc>), nullptr},
    {"lodLevels", 0, offsetof(MeshAssetDesc, lodLevels), sizeof(std::vector<LODLevel>), nullptr},
    {"skinningData", 0, offsetof(MeshAssetDesc, skinningData), sizeof(SkinningData*), nullptr},
  };
}

/**
 * Initialize mesh module.
 * Registers resource factory with ResourceManager and MeshAssetDesc type with TypeRegistry.
 * 
 * @param manager Resource manager instance
 */
void InitializeMeshModule(resource::IResourceManager* manager) {
  if (!manager) {
    return;
  }
  
  // Register resource factory
  manager->RegisterResourceFactory(resource::ResourceType::Mesh, CreateMeshResource);

  // Register MeshAssetDesc type with TypeRegistry
  object::TypeDescriptor meshAssetDescType;
  meshAssetDescType.id = kMeshAssetDescTypeId;
  meshAssetDescType.name = "MeshAssetDesc";
  meshAssetDescType.size = sizeof(MeshAssetDesc);
  meshAssetDescType.properties = g_meshAssetDescProperties;
  meshAssetDescType.propertyCount = sizeof(g_meshAssetDescProperties) / sizeof(g_meshAssetDescProperties[0]);
  meshAssetDescType.baseTypeId = object::kInvalidTypeId;
  meshAssetDescType.createInstance = CreateMeshAssetDesc;

  object::TypeRegistry::RegisterType(meshAssetDescType);
}

}  // namespace mesh
}  // namespace te
