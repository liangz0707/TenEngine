/**
 * @file MeshResource.cpp
 * @brief MeshResource implementation.
 */
#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/mesh/MeshDevice.h>
#include <te/mesh/MeshImporters.h>
#include <te/mesh/detail/mesh_data.hpp>
#include <te/resource/Resource.h>
#include <te/resource/Resource.inl>
#include <te/resource/ResourceManager.h>
#include <te/deviceresource/DeviceResourceManager.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/rhi/resources.hpp>
#include <te/resource/ResourceId.h>
#include <te/core/alloc.h>
#include <cstring>
#include <memory>
#include <string>

// Specialize AssetDescTypeName for MeshAssetDesc (must be in te::resource namespace)
namespace te {
namespace resource {
template<>
struct AssetDescTypeName<mesh::MeshAssetDesc> {
  static const char* Get() { return "MeshAssetDesc"; }
};
}  // namespace resource

namespace mesh {

MeshResource::MeshResource() {
  // Generate GUID for resource ID
  m_resourceId = resource::ResourceId::Generate();
}

MeshResource::~MeshResource() {
  CleanupGPUResources();
  if (m_meshHandle) {
    ReleaseMesh(m_meshHandle);
    m_meshHandle = nullptr;
  }
}

resource::ResourceType MeshResource::GetResourceType() const {
  return resource::ResourceType::Mesh;
}

resource::ResourceId MeshResource::GetResourceId() const {
  return m_resourceId;
}

void MeshResource::Release() {
  if (m_refCount > 0) {
    --m_refCount;
  }
  // When refcount reaches zero, resource may be reclaimed by ResourceManager
  // We don't delete ourselves here
}

bool MeshResource::Load(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager) {
    return false;
  }
  
  // Load AssetDesc (.mesh file)
  std::unique_ptr<MeshAssetDesc> desc = LoadAssetDesc<MeshAssetDesc>(path);
  if (!desc) {
    return false;
  }
  
  // Load data file (.meshdata file)
  // The data file contains vertex data followed by index data
  void* dataFileBuffer = nullptr;
  size_t dataFileSize = 0;
  
  std::string dataPath = GetDataPath(path);
  if (!LoadDataFile(dataPath.c_str(), &dataFileBuffer, &dataFileSize)) {
    return false;
  }
  
  // Verify data file size matches expected size
  size_t expectedSize = desc->vertexDataSize + desc->indexDataSize;
  if (dataFileSize < expectedSize) {
    te::core::Free(dataFileBuffer);
    return false;
  }
  
  // Set pointers in desc (they point to loaded data)
  desc->vertexData = dataFileBuffer;
  if (desc->indexDataSize > 0) {
    desc->indexData = static_cast<uint8_t*>(dataFileBuffer) + desc->vertexDataSize;
  } else {
    desc->indexData = nullptr;
  }
  
  // Create mesh from description
  m_meshHandle = CreateMesh(desc.get());
  if (!m_meshHandle) {
    // Free loaded data
    if (dataFileBuffer) {
      te::core::Free(dataFileBuffer);
    }
    return false;
  }
  
  // Note: dataFileBuffer is now owned by MeshHandle (MeshData)
  // CreateMesh copies the data, so we need to free the original buffer
  te::core::Free(dataFileBuffer);
  
  // Call OnLoadComplete hook
  OnLoadComplete();
  
  return true;
}

bool MeshResource::LoadAsync(char const* path, resource::IResourceManager* manager,
                             resource::LoadCompleteCallback on_done, void* user_data) {
  // Use base class default implementation (uses IThreadPool)
  return IResource::LoadAsync(path, manager, on_done, user_data);
}

bool MeshResource::Save(char const* path, resource::IResourceManager* manager) {
  if (!path || !manager || !m_meshHandle) {
    return false;
  }
  
  // Prepare save data
  OnPrepareSave();
  
  // Create MeshAssetDesc from current mesh
  // This is a simplified implementation - we need to extract data from MeshHandle
  // For now, we'll create a minimal desc
  MeshAssetDesc desc;
  desc.formatVersion = 1;
  desc.debugDescription = "Saved mesh";
  
  // Get data from mesh handle
  detail::MeshData* data = static_cast<detail::MeshData*>(m_meshHandle);
  desc.vertexLayout = data->vertexLayout;
  desc.indexFormat = data->indexFormat;
  desc.vertexData = data->vertexData.get();
  desc.vertexDataSize = data->vertexDataSize;
  desc.indexData = data->indexData.get();
  desc.indexDataSize = data->indexDataSize;
  desc.submeshes = data->submeshes;
  desc.lodLevels = data->lodLevels;
  
  // Save AssetDesc (.mesh file)
  if (!SaveAssetDesc<MeshAssetDesc>(path, &desc)) {
    return false;
  }
  
  // Save data file (.meshdata file)
  std::string dataPath = GetDataPath(path);
  
  // Combine vertex and index data into single buffer
  size_t totalDataSize = desc.vertexDataSize + desc.indexDataSize;
  if (totalDataSize > 0) {
    uint8_t* combinedData = static_cast<uint8_t*>(te::core::Alloc(totalDataSize, alignof(uint8_t)));
    if (!combinedData) {
      return false;
    }
    
    // Copy vertex data
    if (desc.vertexDataSize > 0 && desc.vertexData) {
      std::memcpy(combinedData, desc.vertexData, desc.vertexDataSize);
    }
    
    // Copy index data
    if (desc.indexDataSize > 0 && desc.indexData) {
      std::memcpy(combinedData + desc.vertexDataSize, desc.indexData, desc.indexDataSize);
    }
    
    // Save combined data
    bool saveSuccess = SaveDataFile(dataPath.c_str(), combinedData, totalDataSize);
    te::core::Free(combinedData);
    
    if (!saveSuccess) {
      return false;
    }
  }
  
  return true;
}

bool MeshResource::Import(char const* sourcePath, resource::IResourceManager* manager) {
  if (!sourcePath || !manager) {
    return false;
  }
  
  // Detect file format
  std::string format = DetectFormat(sourcePath);
  
  // Create MeshAssetDesc
  MeshAssetDesc desc;
  bool success = false;
  
  if (format == ".obj") {
    #ifdef TENENGINE_USE_FAST_OBJ
    success = ImportMeshFromFastObj(sourcePath, &desc);
    #endif
    #ifdef TENENGINE_USE_ASSIMP
    if (!success) {
      success = ImportMeshFromAssimp(sourcePath, &desc);
    }
    #endif
  } else if (format == ".gltf" || format == ".glb") {
    #ifdef TENENGINE_USE_CGLTF
    success = ImportMeshFromCgltf(sourcePath, &desc);
    #endif
    #ifdef TENENGINE_USE_ASSIMP
    if (!success) {
      success = ImportMeshFromAssimp(sourcePath, &desc);
    }
    #endif
  } else {
    #ifdef TENENGINE_USE_ASSIMP
    success = ImportMeshFromAssimp(sourcePath, &desc);
    #endif
  }
  
  if (!success || !desc.IsValid()) {
    return false;
  }
  
  // Generate GUID (if not already set)
  if (m_resourceId.IsNull()) {
    m_resourceId = GenerateGUID();
  }
  
  // Determine output path (remove source extension, add .mesh)
  std::string outputPath = sourcePath;
  size_t dotPos = outputPath.find_last_of('.');
  if (dotPos != std::string::npos) {
    outputPath = outputPath.substr(0, dotPos);
  }
  outputPath += ".mesh";
  
  // Save AssetDesc
  if (!SaveAssetDesc<MeshAssetDesc>(outputPath.c_str(), &desc)) {
    // Cleanup allocated data
    if (desc.vertexData) {
      te::core::Free(desc.vertexData);
    }
    if (desc.indexData) {
      te::core::Free(desc.indexData);
    }
    return false;
  }
  
  // Save data file
  std::string dataPath = GetDataPath(outputPath.c_str());
  size_t totalDataSize = desc.vertexDataSize + desc.indexDataSize;
  if (totalDataSize > 0) {
    uint8_t* combinedData = static_cast<uint8_t*>(te::core::Alloc(totalDataSize, alignof(uint8_t)));
    if (!combinedData) {
      if (desc.vertexData) {
        te::core::Free(desc.vertexData);
      }
      if (desc.indexData) {
        te::core::Free(desc.indexData);
      }
      return false;
    }
    
    // Copy vertex data
    if (desc.vertexDataSize > 0 && desc.vertexData) {
      std::memcpy(combinedData, desc.vertexData, desc.vertexDataSize);
    }
    
    // Copy index data
    if (desc.indexDataSize > 0 && desc.indexData) {
      std::memcpy(combinedData + desc.vertexDataSize, desc.indexData, desc.indexDataSize);
    }
    
    // Save combined data
    bool saveSuccess = SaveDataFile(dataPath.c_str(), combinedData, totalDataSize);
    te::core::Free(combinedData);
    
    if (!saveSuccess) {
      if (desc.vertexData) {
        te::core::Free(desc.vertexData);
      }
      if (desc.indexData) {
        te::core::Free(desc.indexData);
      }
      return false;
    }
  }
  
  // Cleanup allocated data
  if (desc.vertexData) {
    te::core::Free(desc.vertexData);
  }
  if (desc.indexData) {
    te::core::Free(desc.indexData);
  }
  
  return true;
}

void MeshResource::EnsureDeviceResources() {
  if (!m_meshHandle || (m_deviceVertexBuffer && m_deviceIndexBuffer)) {
    return;  // Already created or no mesh
  }
  
  if (!m_device) {
    return;  // No device set
  }
  
  // Get data from mesh handle
  detail::MeshData* data = static_cast<detail::MeshData*>(m_meshHandle);
  
  // Create vertex buffer
  if (!m_deviceVertexBuffer && data->vertexDataSize > 0) {
    rhi::BufferDesc rhiBufferDesc{};
    rhiBufferDesc.size = data->vertexDataSize;
    rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex) | static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
    
    m_deviceVertexBuffer = deviceresource::DeviceResourceManager::CreateDeviceBuffer(
      data->vertexData.get(), data->vertexDataSize, rhiBufferDesc, m_device);
    
    if (m_deviceVertexBuffer) {
      data->deviceVertexBuffer = m_deviceVertexBuffer;
    }
  }
  
  // Create index buffer
  if (!m_deviceIndexBuffer && data->indexDataSize > 0) {
    rhi::BufferDesc rhiBufferDesc{};
    rhiBufferDesc.size = data->indexDataSize;
    rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index) | static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
    
    m_deviceIndexBuffer = deviceresource::DeviceResourceManager::CreateDeviceBuffer(
      data->indexData.get(), data->indexDataSize, rhiBufferDesc, m_device);
    
    if (m_deviceIndexBuffer) {
      data->deviceIndexBuffer = m_deviceIndexBuffer;
    }
  }
}

namespace {
  // Context for MeshResource async callback
  struct MeshResourceAsyncContext {
    MeshResource* resource;
    void (*on_done)(void*);
    void* user_data;
  };
}

void MeshResource::EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data) {
  if (!m_meshHandle || (m_deviceVertexBuffer && m_deviceIndexBuffer)) {
    // Already created or no mesh
    if (on_done) {
      on_done(user_data);
    }
    return;
  }
  
  if (!m_device) {
    // No device set
    if (on_done) {
      on_done(user_data);
    }
    return;
  }

  // Create callback context
  MeshResourceAsyncContext* ctx = new MeshResourceAsyncContext();
  if (!ctx) {
    if (on_done) {
      on_done(user_data);
    }
    return;
  }
  ctx->resource = this;
  ctx->on_done = on_done;
  ctx->user_data = user_data;

  // Use MeshDevice async API with callback wrapper
  mesh::EnsureDeviceResourcesAsync(m_meshHandle, m_device, 
    [](void* user_data) {
      MeshResourceAsyncContext* ctx = static_cast<MeshResourceAsyncContext*>(user_data);
      if (!ctx || !ctx->resource) {
        return;
      }

      // Update MeshResource state from MeshData
      if (ctx->resource->m_meshHandle) {
        detail::MeshData* data = static_cast<detail::MeshData*>(ctx->resource->m_meshHandle);
        ctx->resource->m_deviceVertexBuffer = data->deviceVertexBuffer;
        ctx->resource->m_deviceIndexBuffer = data->deviceIndexBuffer;
      }

      // Call user callback
      if (ctx->on_done) {
        ctx->on_done(ctx->user_data);
      }

      // Cleanup context
      delete ctx;
    },
    ctx);
}

void MeshResource::OnLoadComplete() {
  // Mesh-specific initialization after load
  // For now, nothing to do
}

void MeshResource::OnPrepareSave() {
  // Prepare save data
  // For now, nothing to do
}

bool MeshResource::OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) {
  // This method is not used in the current implementation.
  // Import is handled directly in MeshResource::Import.
  (void)sourcePath;
  (void)outData;
  (void)outSize;
  return false;
}

void* MeshResource::OnCreateAssetDesc() {
  // This method is not used in the current implementation.
  // Import is handled directly in MeshResource::Import.
  return nullptr;
}

void const* MeshResource::GetVertexData() const {
  if (!m_meshHandle) {
    return nullptr;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(m_meshHandle);
  return data->vertexData.get();
}

size_t MeshResource::GetVertexDataSize() const {
  if (!m_meshHandle) {
    return 0;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(m_meshHandle);
  return data->vertexDataSize;
}

void const* MeshResource::GetIndexData() const {
  if (!m_meshHandle) {
    return nullptr;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(m_meshHandle);
  return data->indexData.get();
}

size_t MeshResource::GetIndexDataSize() const {
  if (!m_meshHandle) {
    return 0;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(m_meshHandle);
  return data->indexDataSize;
}

void MeshResource::CleanupGPUResources() {
  if (m_deviceVertexBuffer && m_device) {
    deviceresource::DeviceResourceManager::DestroyDeviceBuffer(m_deviceVertexBuffer, m_device);
    m_deviceVertexBuffer = nullptr;
  }
  if (m_deviceIndexBuffer && m_device) {
    deviceresource::DeviceResourceManager::DestroyDeviceBuffer(m_deviceIndexBuffer, m_device);
    m_deviceIndexBuffer = nullptr;
  }
}

}  // namespace mesh
}  // namespace te
