# Contract: 030-DeviceResourceManager Module Public API

## Applicable Module

- **Implementor**: 030-DeviceResourceManager (L2; unified GPU resource manager, command list pool, staging buffer management, sync/async GPU resource creation)
- **Spec**: `docs/module-specs/030-device-resource-manager.md`
- **Dependencies**: 001-Core, 008-RHI

## Consumers

- **028-Texture**: Creates GPU texture resources (via EnsureDeviceResources call)
- **012-Mesh**: Creates GPU buffer resources (subsequent phase)
- **011-Material**: Binds GPU resources (subsequent phase)
- **019-PipelineCore**: PrepareRenderResources unified call (optional, via each resource module indirectly)

---

## Module Positioning and Core Responsibilities

030-DeviceResourceManager provides a **unified GPU resource manager**, responsible for:
- **Unified management of all GPU resource types**: Texture, Buffer, etc.
- **Command list management**: Command list pool (for async upload), managed per IDevice
- **Data upload**: Staging buffer management, supports sync and async upload
- **Resource barriers**: Unified handling of resource state transitions
- **Synchronization**: Fence management, supports async operation completion callbacks
- **Lifecycle management**: GPU resources bound to RResource, delayed destruction

030 **does not handle Shader/PSO**: Shader compilation is handled by 010-Shader (CPU operation), PSO creation is done by Material or Pipeline directly calling 008-RHI.

030 **uses data-oriented interface**: Does not directly depend on 028-Texture or 012-Mesh concrete types, but accepts raw data parameters (pixelData, bufferData, etc.), avoiding circular dependencies.

---

## Capabilities

### 0. Operation Status Types

**Operation Status Enum**:
```cpp
enum class ResourceOperationStatus {
  Pending,     // Request created, waiting to start
  Uploading,   // Currently uploading data (allocating staging buffer, recording commands)
  Submitted,   // Commands submitted to GPU, waiting for fence
  Completed,   // Operation completed successfully
  Failed,      // Operation failed
  Cancelled    // Operation was cancelled
};
```

**Operation Handle Type**:
```cpp
using ResourceOperationHandle = void*;  // Opaque handle
```
- Returned by `CreateDeviceTextureAsync` or `CreateDeviceBufferAsync`
- Used for status query, progress query, and cancellation

### 1. DeviceResourceManager Static Class

DeviceResourceManager provides static methods for unified GPU resource creation, no instantiation needed.

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| **DeviceResourceManager** | GPU resource manager (static method class) | Global static, no instantiation needed |

#### 1.1 Texture Resource Creation

**Sync create GPU texture**:
```cpp
static rhi::ITexture* CreateDeviceTexture(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device);
```
- **Parameters**:
  - `pixelData`: Pixel data pointer (RGBA8 format)
  - `pixelDataSize`: Pixel data size (bytes)
  - `textureDesc`: RHI texture description (width, height, format, etc.)
  - `device`: RHI device
- **Returns**: GPU texture handle, `nullptr` on failure
- **Description**: Sync create GPU texture and upload data, set resource barrier

**Async create GPU texture**:
```cpp
static ResourceOperationHandle CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data);
```
- **Parameters**:
  - `pixelData`: Pixel data pointer
  - `pixelDataSize`: Pixel data size (bytes)
  - `textureDesc`: RHI texture description
  - `device`: RHI device
  - `callback`: Completion callback function
  - `user_data`: User data
- **Returns**: Operation handle (`ResourceOperationHandle`) for status query and cancellation; `nullptr` on failure
- **Description**: Uses command list pool for async GPU texture creation, callback executes on agreed thread

**Update GPU texture data**:
```cpp
static bool UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    void const* data,
    size_t size,
    rhi::TextureDesc const& textureDesc);
```
- **Parameters**:
  - `texture`: GPU texture handle
  - `device`: RHI device
  - `data`: Source data
  - `size`: Data size (bytes)
  - `textureDesc`: Texture description (width, height, format, etc.) to determine update region
- **Returns**: `true` on success, `false` on failure
- **Description**: Updates GPU texture data, needs texture description to determine update region

**Destroy GPU texture**:
```cpp
static void DestroyDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device);
```
- **Parameters**:
  - `texture`: GPU texture handle
  - `device`: RHI device
- **Description**: Destroys GPU texture resource

#### 1.2 Buffer Resource Creation

**Sync create GPU buffer**:
```cpp
static rhi::IBuffer* CreateDeviceBuffer(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device);
```
- **Parameters**:
  - `data`: Buffer data pointer (vertex data or index data)
  - `dataSize`: Data size (bytes)
  - `bufferDesc`: RHI buffer description (size, usage, etc.)
  - `device`: RHI device
- **Returns**: GPU buffer handle, `nullptr` on failure
- **Description**: Creates GPU buffer from raw data and uploads data, sets resource barrier. Callers (e.g. 012-Mesh) should call this from EnsureDeviceResources after obtaining data.

**Async create GPU buffer**:
```cpp
static ResourceOperationHandle CreateDeviceBufferAsync(
    void const* data,
    size_t dataSize,
    rhi::BufferDesc const& bufferDesc,
    rhi::IDevice* device,
    void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
    void* user_data);
```
- **Parameters**:
  - `data`: Buffer data pointer
  - `dataSize`: Data size (bytes)
  - `bufferDesc`: RHI buffer description
  - `device`: RHI device
  - `callback`: Completion callback function
  - `user_data`: User data
- **Returns**: Operation handle (`ResourceOperationHandle`) for status query and cancellation; `nullptr` on failure
- **Description**: Uses command list pool for async GPU buffer creation, callback executes on agreed thread

**Destroy GPU buffer**:
```cpp
static void DestroyDeviceBuffer(
    rhi::IBuffer* buffer,
    rhi::IDevice* device);
```
- **Parameters**:
  - `buffer`: GPU buffer handle
  - `device`: RHI device
- **Description**: Destroys GPU buffer resource

#### 1.3 Operation Status Query

**Query operation status**:
```cpp
static ResourceOperationStatus GetOperationStatus(ResourceOperationHandle handle);
```
- **Parameters**:
  - `handle`: Operation handle (returned by `CreateDeviceTextureAsync` or `CreateDeviceBufferAsync`)
- **Returns**: Current operation status (`Pending`, `Uploading`, `Submitted`, `Completed`, `Failed`, `Cancelled`)
- **Description**: Thread-safe, can query async operation status at any time

**Query operation progress**:
```cpp
static float GetOperationProgress(ResourceOperationHandle handle);
```
- **Parameters**:
  - `handle`: Operation handle
- **Returns**: Progress value (0.0 ~ 1.0), 0.0 means started, 1.0 means completed
- **Description**: Thread-safe, returns current operation completion progress

**Cancel operation**:
```cpp
static void CancelOperation(ResourceOperationHandle handle);
```
- **Parameters**:
  - `handle`: Operation handle
- **Description**: Cancels incomplete operation. Callback will still trigger with `success` parameter as `false`. Thread-safe

#### 1.4 Cleanup Operations

**Cleanup device resources**:
```cpp
static void CleanupDevice(rhi::IDevice* device);
```
- **Parameters**:
  - `device`: RHI device
- **Description**: Cleans up command list pool and staging buffers for specified device, should be called before IDevice destruction

### 2. CommandListPool Class (Public Interface)

CommandListPool provides command list pool management for efficient async GPU resource creation.

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| **CommandListPool** | Command list pool (managed per IDevice) | Bound to IDevice |

**Constructor**:
```cpp
explicit CommandListPool(rhi::IDevice* device);
```

**Acquire command list**:
```cpp
rhi::ICommandList* Acquire();
```
- **Returns**: Command list pointer, creates new one if pool is empty
- **Description**: Thread-safe

**Release command list**:
```cpp
void Release(rhi::ICommandList* cmd);
```
- **Parameters**:
  - `cmd`: Command list pointer
- **Description**: Returns command list to pool for reuse

**Clear pool**:
```cpp
void Clear();
```
- **Description**: Clears all command lists

### 3. StagingBufferManager Class (Public Interface)

StagingBufferManager provides staging buffer management for efficient GPU data upload.

| Name | Semantics | Lifecycle |
|------|-----------|-----------|
| **StagingBufferManager** | Staging buffer manager (managed per IDevice) | Bound to IDevice |

**Constructor**:
```cpp
explicit StagingBufferManager(rhi::IDevice* device);
```

**Allocate staging buffer**:
```cpp
rhi::IBuffer* Allocate(size_t size);
```
- **Parameters**:
  - `size`: Requested size (bytes)
- **Returns**: Staging buffer pointer, `nullptr` on allocation failure
- **Description**: Allocates at least the requested size staging buffer, thread-safe

**Release staging buffer**:
```cpp
void Release(rhi::IBuffer* buffer);
```
- **Parameters**:
  - `buffer`: Staging buffer pointer
- **Description**: Returns staging buffer to pool

**Clear all buffers**:
```cpp
void Clear();
```
- **Description**: Clears all staging buffers

---

## Types and Callbacks

### Callback Types

| Name | Type Definition | Description |
|------|-----------------|-------------|
| **TextureCreateCallback** | `void (*)(rhi::ITexture* texture, bool success, void* user_data)` | Texture creation completion callback |
| **BufferCreateCallback** | `void (*)(rhi::IBuffer* buffer, bool success, void* user_data)` | Buffer creation completion callback |

---

## Namespace

All public interfaces are in `te::deviceresource` namespace.

---

## Version / ABI

- Follows Constitution: Public API versioning; breaking changes increment MAJOR.

---

## Constraints

- Must be used after Core and RHI initialization.
- Caller must call `CleanupDevice` before IDevice destruction to release resources.
- Command list pool and staging buffers managed per IDevice, thread-safe.
- Async operation callbacks execute on agreed thread (default main thread, consistent with 013-Resource LoadCompleteCallback).
- **Data-oriented interface**: All resource creation interfaces (Texture, Buffer) accept raw data parameters, do not directly depend on 028-Texture or 012-Mesh concrete types, avoiding circular dependencies.
  - 028-Texture calls `CreateDeviceTexture` in `EnsureDeviceResources`, passing `GetPixelData()` etc. data
  - 012-Mesh calls `CreateDeviceBuffer` in `EnsureDeviceResources`, passing `GetVertexData()` etc. data

---

## Usage Examples

### Sync Create Texture

```cpp
// Get data from TextureResource
void const* pixelData = textureResource->GetPixelData();
size_t pixelDataSize = textureResource->GetPixelDataSize();
rhi::TextureDesc textureDesc = textureResource->GetRHITextureDesc();

// Create GPU texture
rhi::ITexture* gpuTexture = te::deviceresource::DeviceResourceManager::CreateDeviceTexture(
    pixelData, pixelDataSize, textureDesc, device);

if (gpuTexture) {
    textureResource->SetDeviceTexture(gpuTexture);
}
```

### Async Create Texture

```cpp
void OnTextureCreated(rhi::ITexture* texture, bool success, void* user_data) {
    TextureResource* resource = static_cast<TextureResource*>(user_data);
    if (success && texture) {
        resource->SetDeviceTexture(texture);
    }
}

te::deviceresource::DeviceResourceManager::CreateDeviceTextureAsync(
    pixelData, pixelDataSize, textureDesc, device,
    OnTextureCreated, textureResource);
```

### Sync Create Buffer

```cpp
// Get data from MeshResource
void const* vertexData = meshResource->GetVertexData();
size_t vertexDataSize = meshResource->GetVertexDataSize();
rhi::BufferDesc vertexBufferDesc{};
vertexBufferDesc.size = vertexDataSize;
vertexBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex);

// Create GPU vertex buffer
rhi::IBuffer* vertexBuffer = te::deviceresource::DeviceResourceManager::CreateDeviceBuffer(
    vertexData, vertexDataSize, vertexBufferDesc, device);

if (vertexBuffer) {
    meshResource->SetDeviceVertexBuffer(vertexBuffer);
}

// Similarly create index buffer
void const* indexData = meshResource->GetIndexData();
size_t indexDataSize = meshResource->GetIndexDataSize();
rhi::BufferDesc indexBufferDesc{};
indexBufferDesc.size = indexDataSize;
indexBufferDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index);

rhi::IBuffer* indexBuffer = te::deviceresource::DeviceResourceManager::CreateDeviceBuffer(
    indexData, indexDataSize, indexBufferDesc, device);

if (indexBuffer) {
    meshResource->SetDeviceIndexBuffer(indexBuffer);
}
```

### Cleanup Device Resources

```cpp
// Call before IDevice destruction
te::deviceresource::DeviceResourceManager::CleanupDevice(device);
```

---

## Change Log

| Date | Change |
|------|--------|
| 2026-02-10 | Initial contract creation |
| 2026-02-11 | Added CommandListPool and StagingBufferManager public interfaces; clarified data-oriented interface design |
| 2026-02-22 | Updated to match actual implementation; removed 013-Resource from dependencies (030 uses data-oriented interface, no direct dependency on 013-Resource types) |
