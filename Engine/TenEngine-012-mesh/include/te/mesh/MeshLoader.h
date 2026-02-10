/**
 * @file MeshLoader.h
 * @brief Mesh resource loader (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_LOADER_H
#define TE_MESH_MESH_LOADER_H

#include <te/resource/ResourceManager.h>

namespace te {
namespace mesh {

/**
 * MeshResourceLoader: creates MeshResource from payload.
 * Implements IResourceLoader interface for ResourceType::Mesh.
 */
class MeshResourceLoader {
 public:
  /**
   * Create resource from payload.
   * Called by ResourceManager when loading Mesh resource type.
   * 
   * @param type Resource type (must be ResourceType::Mesh)
   * @param payload MeshAssetDesc* (cast from void*)
   * @param manager Resource manager
   * @return IResource* (MeshResource instance), or nullptr on failure
   */
  static resource::IResource* CreateFromPayload(resource::ResourceType type, void* payload,
                                                 resource::IResourceManager* manager);
};

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_LOADER_H
