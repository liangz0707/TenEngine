/**
 * @file MeshResource.h
 * @brief MeshResource class implementing IMeshResource/IResource (contract: specs/_contracts/012-mesh-ABI.md).
 */
#ifndef TE_MESH_MESH_RESOURCE_H
#define TE_MESH_MESH_RESOURCE_H

#include <te/resource/MeshResource.h>
#include <te/resource/ResourceTypes.h>
#include <te/resource/ResourceId.h>
#include <te/mesh/Mesh.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/rhi/resources.hpp>
#include <te/rhi/device.hpp>
#include <memory>
#include <string>

namespace te {
namespace resource {
class IResourceManager;
}

namespace mesh {

/**
 * MeshResource: implements IMeshResource (inherits IResource).
 * Manages mesh data lifecycle: Load, Save, Import, EnsureDeviceResources.
 */
class MeshResource : public resource::IMeshResource {
 public:
  /**
   * Constructor.
   */
  MeshResource();
  
  /**
   * Destructor.
   * Releases mesh handle and GPU resources.
   */
  ~MeshResource() override;
  
  // IResource pure virtual functions
  resource::ResourceType GetResourceType() const override;
  resource::ResourceId GetResourceId() const override;
  void Release() override;
  
  // IResource virtual functions (override for mesh-specific behavior)
  bool Load(char const* path, resource::IResourceManager* manager) override;
  bool LoadAsync(char const* path, resource::IResourceManager* manager,
                 resource::LoadCompleteCallback on_done, void* user_data) override;
  bool Save(char const* path, resource::IResourceManager* manager) override;
  bool Import(char const* sourcePath, resource::IResourceManager* manager) override;
  void EnsureDeviceResources() override;
  void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) override;
  
  // Mesh-specific accessors
  /**
   * Get mesh handle.
   * @return Mesh handle, or nullptr if not loaded
   */
  MeshHandle GetMeshHandle() const { return m_meshHandle; }
  
  /**
   * Get vertex data pointer.
   * @return Vertex data pointer, or nullptr if not loaded
   */
  void const* GetVertexData() const;
  
  /**
   * Get vertex data size.
   * @return Vertex data size in bytes
   */
  size_t GetVertexDataSize() const;
  
  /**
   * Get index data pointer.
   * @return Index data pointer, or nullptr if not loaded
   */
  void const* GetIndexData() const;
  
  /**
   * Get index data size.
   * @return Index data size in bytes
   */
  size_t GetIndexDataSize() const;
  
  /**
   * Set device for GPU resource creation.
   * @param device RHI device
   */
  void SetDevice(rhi::IDevice* device) { m_device = device; }
  
  /**
   * Get device vertex buffer handle.
   * @return Vertex buffer handle, or nullptr if not created
   */
  rhi::IBuffer* GetDeviceVertexBuffer() const { return m_deviceVertexBuffer; }
  
  /**
   * Get device index buffer handle.
   * @return Index buffer handle, or nullptr if not created
   */
  rhi::IBuffer* GetDeviceIndexBuffer() const { return m_deviceIndexBuffer; }
  
  /**
   * Set device vertex buffer handle.
   * Called by EnsureDeviceResources after GPU buffer creation.
   */
  void SetDeviceVertexBuffer(rhi::IBuffer* buffer) { m_deviceVertexBuffer = buffer; }
  
  /**
   * Set device index buffer handle.
   * Called by EnsureDeviceResources after GPU buffer creation.
   */
  void SetDeviceIndexBuffer(rhi::IBuffer* buffer) { m_deviceIndexBuffer = buffer; }
  
  /**
   * Set mesh handle.
   * Called by MeshLoader::CreateFromPayload.
   * @param handle Mesh handle (takes ownership)
   */
  void SetMeshHandle(MeshHandle handle) { m_meshHandle = handle; }

 protected:
  // IResource protected virtual functions
  void OnLoadComplete() override;
  void OnPrepareSave() override;
  bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) override;
  void* OnCreateAssetDesc() override;

 private:
  MeshHandle m_meshHandle = nullptr;
  resource::ResourceId m_resourceId;
  rhi::IDevice* m_device = nullptr;
  rhi::IBuffer* m_deviceVertexBuffer = nullptr;
  rhi::IBuffer* m_deviceIndexBuffer = nullptr;
  uint32_t m_refCount = 1;
  
  // Helper methods
  void CleanupGPUResources();
};

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_MESH_RESOURCE_H
