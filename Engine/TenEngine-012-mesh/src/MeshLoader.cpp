/**
 * @file MeshLoader.cpp
 * @brief Mesh resource loader implementation.
 */
#include <te/mesh/MeshLoader.h>
#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>

namespace te {
namespace mesh {

resource::IResource* MeshResourceLoader::CreateFromPayload(resource::ResourceType type, void* payload,
                                                            resource::IResourceManager* manager) {
  if (type != resource::ResourceType::Mesh || !payload || !manager) {
    return nullptr;
  }
  
  // Cast payload to MeshAssetDesc*
  MeshAssetDesc* desc = static_cast<MeshAssetDesc*>(payload);
  if (!desc || !desc->IsValid()) {
    return nullptr;
  }
  
  // Create mesh handle
  MeshHandle meshHandle = CreateMesh(desc);
  if (!meshHandle) {
    return nullptr;
  }
  
  // Create MeshResource instance
  MeshResource* resource = new MeshResource();
  if (!resource) {
    ReleaseMesh(meshHandle);
    return nullptr;
  }
  
  // Set mesh handle (takes ownership)
  resource->SetMeshHandle(meshHandle);
  
  return resource;
}

}  // namespace mesh
}  // namespace te
