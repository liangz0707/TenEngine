/**
 * @file MeshDevice.cpp
 * @brief GPU device resource management implementation.
 */
#include <te/mesh/MeshDevice.h>
#include <te/mesh/detail/mesh_data.hpp>
#include <te/deviceresource/DeviceResourceManager.h>
#include <te/rhi/resources.hpp>
#include <te/rhi/device.hpp>

namespace te {
namespace mesh {

bool EnsureDeviceResources(MeshHandle h, rhi::IDevice* device) {
  if (!h || !device) {
    return false;
  }
  
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  
  // Check if already created
  if (data->deviceVertexBuffer && data->deviceIndexBuffer) {
    return true;
  }
  
  // Create vertex buffer
  if (!data->deviceVertexBuffer && data->vertexDataSize > 0) {
    rhi::BufferDesc rhiBufferDesc{};
    rhiBufferDesc.size = data->vertexDataSize;
    rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex) | static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
    
    data->deviceVertexBuffer = deviceresource::DeviceResourceManager::CreateDeviceBuffer(
      data->vertexData.get(), data->vertexDataSize, rhiBufferDesc, device);
    
    if (!data->deviceVertexBuffer) {
      return false;
    }
  }
  
  // Create index buffer
  if (!data->deviceIndexBuffer && data->indexDataSize > 0) {
    rhi::BufferDesc rhiBufferDesc{};
    rhiBufferDesc.size = data->indexDataSize;
    rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index) | static_cast<uint32_t>(rhi::BufferUsage::CopyDst);
    
    data->deviceIndexBuffer = deviceresource::DeviceResourceManager::CreateDeviceBuffer(
      data->indexData.get(), data->indexDataSize, rhiBufferDesc, device);
    
    if (!data->deviceIndexBuffer) {
      // Cleanup vertex buffer on failure
      if (data->deviceVertexBuffer) {
        deviceresource::DeviceResourceManager::DestroyDeviceBuffer(data->deviceVertexBuffer, device);
        data->deviceVertexBuffer = nullptr;
      }
      return false;
    }
  }
  
  return true;
}

namespace {
  // Context for async resource creation
  struct AsyncResourceContext {
    detail::MeshData* data;
    rhi::IDevice* device;
    void (*on_done)(void*);
    void* user_data;
    bool vertexBufferCreated;
    bool indexBufferCreated;
    bool hasError;
  };

  // Callback for index buffer creation completion
  void OnIndexBufferCreated(rhi::IBuffer* buffer, bool success, void* user_data) {
    AsyncResourceContext* ctx = static_cast<AsyncResourceContext*>(user_data);
    if (!ctx) {
      return;
    }

    if (!success || !buffer) {
      ctx->hasError = true;
      // Cleanup vertex buffer if it was created
      if (ctx->vertexBufferCreated && ctx->data->deviceVertexBuffer) {
        deviceresource::DeviceResourceManager::DestroyDeviceBuffer(
          ctx->data->deviceVertexBuffer, ctx->device);
        ctx->data->deviceVertexBuffer = nullptr;
      }
      if (ctx->on_done) {
        ctx->on_done(ctx->user_data);
      }
      delete ctx;
      return;
    }

    ctx->data->deviceIndexBuffer = buffer;
    ctx->indexBufferCreated = true;

    // Both buffers created successfully
    if (ctx->on_done) {
      ctx->on_done(ctx->user_data);
    }
    delete ctx;
  }

  // Callback for vertex buffer creation completion
  void OnVertexBufferCreated(rhi::IBuffer* buffer, bool success, void* user_data) {
    AsyncResourceContext* ctx = static_cast<AsyncResourceContext*>(user_data);
    if (!ctx) {
      return;
    }

    if (!success || !buffer) {
      ctx->hasError = true;
      if (ctx->on_done) {
        ctx->on_done(ctx->user_data);
      }
      delete ctx;
      return;
    }

    ctx->data->deviceVertexBuffer = buffer;
    ctx->vertexBufferCreated = true;

    // Create index buffer asynchronously
    if (ctx->data->indexDataSize > 0 && !ctx->data->deviceIndexBuffer) {
      rhi::BufferDesc rhiBufferDesc{};
      rhiBufferDesc.size = ctx->data->indexDataSize;
      rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index) | 
                            static_cast<uint32_t>(rhi::BufferUsage::CopyDst);

      deviceresource::DeviceResourceManager::CreateDeviceBufferAsync(
        ctx->data->indexData.get(), ctx->data->indexDataSize, rhiBufferDesc, ctx->device,
        OnIndexBufferCreated, ctx);
    } else {
      // Index buffer already exists or not needed
      if (ctx->on_done) {
        ctx->on_done(ctx->user_data);
      }
      delete ctx;
    }
  }
}

void EnsureDeviceResourcesAsync(MeshHandle h, rhi::IDevice* device,
                                void (*on_done)(void*), void* user_data) {
  if (!h || !device) {
    if (on_done) {
      on_done(user_data);
    }
    return;
  }
  
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  
  // Check if already created
  if (data->deviceVertexBuffer && data->deviceIndexBuffer) {
    if (on_done) {
      on_done(user_data);
    }
    return;
  }

  // Create async context
  AsyncResourceContext* ctx = new AsyncResourceContext();
  if (!ctx) {
    if (on_done) {
      on_done(user_data);
    }
    return;
  }

  ctx->data = data;
  ctx->device = device;
  ctx->on_done = on_done;
  ctx->user_data = user_data;
  ctx->vertexBufferCreated = false;
  ctx->indexBufferCreated = false;
  ctx->hasError = false;

  // Create vertex buffer asynchronously first
  if (!data->deviceVertexBuffer && data->vertexDataSize > 0) {
    rhi::BufferDesc rhiBufferDesc{};
    rhiBufferDesc.size = data->vertexDataSize;
    rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex) | 
                          static_cast<uint32_t>(rhi::BufferUsage::CopyDst);

    deviceresource::ResourceOperationHandle handle = 
      deviceresource::DeviceResourceManager::CreateDeviceBufferAsync(
        data->vertexData.get(), data->vertexDataSize, rhiBufferDesc, device,
        OnVertexBufferCreated, ctx);

    if (!handle) {
      // Failed to start async operation
      delete ctx;
      if (on_done) {
        on_done(user_data);
      }
      return;
    }
  } else {
    // Vertex buffer already exists, create index buffer directly
    if (!data->deviceIndexBuffer && data->indexDataSize > 0) {
      rhi::BufferDesc rhiBufferDesc{};
      rhiBufferDesc.size = data->indexDataSize;
      rhiBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index) | 
                            static_cast<uint32_t>(rhi::BufferUsage::CopyDst);

      deviceresource::ResourceOperationHandle handle = 
        deviceresource::DeviceResourceManager::CreateDeviceBufferAsync(
          data->indexData.get(), data->indexDataSize, rhiBufferDesc, device,
          OnIndexBufferCreated, ctx);

      if (!handle) {
        delete ctx;
        if (on_done) {
          on_done(user_data);
        }
        return;
      }
    } else {
      // Both buffers already exist
      delete ctx;
      if (on_done) {
        on_done(user_data);
      }
    }
  }
}

rhi::IBuffer* GetVertexBufferHandle(MeshHandle h) {
  if (!h) {
    return nullptr;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  return data->deviceVertexBuffer;
}

rhi::IBuffer* GetIndexBufferHandle(MeshHandle h) {
  if (!h) {
    return nullptr;
  }
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  return data->deviceIndexBuffer;
}

}  // namespace mesh
}  // namespace te
