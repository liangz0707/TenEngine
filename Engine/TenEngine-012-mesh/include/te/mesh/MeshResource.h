/**
 * @file MeshResource.h
 * @brief MeshResource class implementing IMeshResource/IResource (contract: specs/_contracts/012-mesh-ABI.md). CPU-only.
 */
#ifndef TE_MESH_MESH_RESOURCE_H
#define TE_MESH_MESH_RESOURCE_H

#include <te/resource/MeshResource.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceId.h>
#include <te/mesh/Mesh.h>
#include <te/mesh/MeshAssetDesc.h>
#include <memory>
#include <string>

namespace te {
namespace resource {
class IResourceManager;
}

namespace mesh {

/**
 * MeshResource: implements IMeshResource (inherits IResource).
 * Manages mesh data lifecycle: Load, Save, Import. CPU-only; no GPU buffers.
 */
class MeshResource : public resource::IMeshResource {
 public:
  MeshResource();
  ~MeshResource() override;

  resource::ResourceType GetResourceType() const override;
  resource::ResourceId GetResourceId() const override;
  void Release() override;

  bool Load(char const* path, resource::IResourceManager* manager) override;
  bool LoadAsync(char const* path, resource::IResourceManager* manager,
                 resource::LoadCompleteCallback on_done, void* user_data) override;
  bool Save(char const* path, resource::IResourceManager* manager) override;
  bool Import(char const* sourcePath, resource::IResourceManager* manager) override;
  /** True when mesh is loaded (CPU data ready). */
  bool IsDeviceReady() const override;

  MeshHandle GetMeshHandle() const { return m_meshHandle; }
  void const* GetVertexData() const;
  size_t GetVertexDataSize() const;
  void const* GetIndexData() const;
  size_t GetIndexDataSize() const;

  /** Set mesh handle. Called by MeshLoader::CreateFromPayload. */
  void SetMeshHandle(MeshHandle handle);

 protected:
  bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override;
  void* OnCreateAssetDesc() override;

 private:
  MeshHandle m_meshHandle = nullptr;
  resource::ResourceId m_resourceId;
  uint32_t m_refCount = 1;
};

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_RESOURCE_H
