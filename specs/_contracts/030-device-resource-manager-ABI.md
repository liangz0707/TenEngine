# 030-DeviceResourceManager Module ABI

- **Contract**: [030-device-resource-manager-public-api.md](./030-device-resource-manager-public-api.md) (capabilities and type descriptions)
- **This file**: 030-DeviceResourceManager public ABI explicit table.
- **Naming**: Member methods use **PascalCase**; all methods show **complete function signatures** in Description column.

## ABI Table

Column definitions: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | class | GPU resource manager (static methods) | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager | Provides static methods for unified GPU resource creation (Texture, Buffer, etc.) |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Sync create GPU texture | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceTexture | `static rhi::ITexture* CreateDeviceTexture(void const* pixelData, size_t pixelDataSize, rhi::TextureDesc const& textureDesc, rhi::IDevice* device);` Creates GPU texture from pixel data, returns ITexture*, nullptr on failure |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Async create GPU texture | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceTextureAsync | `static ResourceOperationHandle CreateDeviceTextureAsync(void const* pixelData, size_t pixelDataSize, rhi::TextureDesc const& textureDesc, rhi::IDevice* device, void (*callback)(rhi::ITexture* texture, bool success, void* user_data), void* user_data);` Async GPU texture creation, returns operation handle for status query, callback on agreed thread |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Update GPU texture data | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::UpdateDeviceTexture | `static bool UpdateDeviceTexture(rhi::ITexture* texture, rhi::IDevice* device, void const* data, size_t size, rhi::TextureDesc const& textureDesc);` Updates GPU texture data, needs texture description for update region |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Destroy GPU texture | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::DestroyDeviceTexture | `static void DestroyDeviceTexture(rhi::ITexture* texture, rhi::IDevice* device);` Destroys GPU texture resource |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Sync create GPU buffer | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceBuffer | `static rhi::IBuffer* CreateDeviceBuffer(void const* data, size_t dataSize, rhi::BufferDesc const& bufferDesc, rhi::IDevice* device);` Creates GPU buffer from raw data (vertex/index), returns IBuffer*, nullptr on failure; callers (e.g. 012-Mesh) should call from EnsureDeviceResources |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Async create GPU buffer | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceBufferAsync | `static ResourceOperationHandle CreateDeviceBufferAsync(void const* data, size_t dataSize, rhi::BufferDesc const& bufferDesc, rhi::IDevice* device, void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data), void* user_data);` Async GPU buffer creation, returns operation handle for status query, callback on agreed thread |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Query operation status | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::GetOperationStatus | `static ResourceOperationStatus GetOperationStatus(ResourceOperationHandle handle);` Query async operation status, thread-safe |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Query operation progress | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::GetOperationProgress | `static float GetOperationProgress(ResourceOperationHandle handle);` Query async operation progress (0.0~1.0), thread-safe |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Cancel operation | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CancelOperation | `static void CancelOperation(ResourceOperationHandle handle);` Cancel incomplete async operation, callback still triggers, thread-safe |
| 030-DeviceResourceManager | te::deviceresource | -- | enum | Operation status | te/deviceresource/ResourceOperationTypes.h | ResourceOperationStatus | `enum class ResourceOperationStatus { Pending, Uploading, Submitted, Completed, Failed, Cancelled };` Async operation status enum |
| 030-DeviceResourceManager | te::deviceresource | -- | type alias/handle | Operation handle | te/deviceresource/ResourceOperationTypes.h | ResourceOperationHandle | `using ResourceOperationHandle = void*;` Opaque handle, returned by async creation interfaces |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Destroy GPU buffer | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::DestroyDeviceBuffer | `static void DestroyDeviceBuffer(rhi::IBuffer* buffer, rhi::IDevice* device);` Destroys GPU buffer resource |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | static method | Cleanup device resources | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CleanupDevice | `static void CleanupDevice(rhi::IDevice* device);` Cleans up command list pool and staging buffers for device, call before IDevice destruction |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | class | Command list pool | te/deviceresource/CommandListPool.h | CommandListPool | Command list pool management for efficient async GPU resource creation, managed per IDevice, thread-safe |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | constructor | Create command list pool | te/deviceresource/CommandListPool.h | CommandListPool::CommandListPool | `explicit CommandListPool(rhi::IDevice* device);` Creates command list pool |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | method | Acquire command list | te/deviceresource/CommandListPool.h | CommandListPool::Acquire | `rhi::ICommandList* Acquire();` Gets command list from pool, creates new if empty, thread-safe |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | method | Release command list | te/deviceresource/CommandListPool.h | CommandListPool::Release | `void Release(rhi::ICommandList* cmd);` Returns command list to pool for reuse |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | method | Clear pool | te/deviceresource/CommandListPool.h | CommandListPool::Clear | `void Clear();` Clears all command lists |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | class | Staging buffer manager | te/deviceresource/StagingBufferManager.h | StagingBufferManager | Staging buffer management for efficient GPU data upload, managed per IDevice, thread-safe |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | constructor | Create staging buffer manager | te/deviceresource/StagingBufferManager.h | StagingBufferManager::StagingBufferManager | `explicit StagingBufferManager(rhi::IDevice* device);` Creates staging buffer manager |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | method | Allocate staging buffer | te/deviceresource/StagingBufferManager.h | StagingBufferManager::Allocate | `rhi::IBuffer* Allocate(size_t size);` Allocates at least requested size staging buffer, nullptr on failure, thread-safe |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | method | Release staging buffer | te/deviceresource/StagingBufferManager.h | StagingBufferManager::Release | `void Release(rhi::IBuffer* buffer);` Returns staging buffer to pool |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | method | Clear all buffers | te/deviceresource/StagingBufferManager.h | StagingBufferManager::Clear | `void Clear();` Clears all staging buffers |

---

## Internal Implementation (Not Public ABI)

The following classes are internal implementation, not exposed as public ABI:

| Module | Namespace | Class | Description |
|--------|-----------|-------|-------------|
| 030-DeviceResourceManager | te::deviceresource::internal | TextureDeviceImpl | Texture GPU resource creation implementation (internal) |
| 030-DeviceResourceManager | te::deviceresource::internal | ResourceUploadHelper | Resource upload helper class (internal) |
| 030-DeviceResourceManager | te::deviceresource::internal | DeviceResources | Device resource management structure (internal) |
| 030-DeviceResourceManager | te::deviceresource::internal | AsyncOperationContext | Async operation context (internal) |

---

## Dependencies

### Upstream Dependencies

| Module | Dependency Content | Purpose |
|--------|-------------------|---------|
| **001-Core** | Log, memory allocation | Logging, memory management |
| **008-RHI** | IDevice, ITexture, IBuffer, ICommandList, IFence, IQueue, ResourceState, TextureDesc, BufferUsage | GPU resource creation, command lists, synchronization |

### Downstream Dependencies

| Module | Usage Content | Purpose |
|--------|---------------|---------|
| **028-Texture** | CreateDeviceTexture, CreateDeviceTextureAsync | Creates GPU texture resources |
| **012-Mesh** | CreateDeviceBuffer, CreateDeviceBufferAsync | Creates GPU buffer resources (subsequent phase) |

---

## Design Notes

### Data-Oriented Interface

030 uses data-oriented interface design, avoiding circular dependencies:
- `CreateDeviceTexture` accepts raw pixel data and texture description, does not directly depend on `TextureResource` type
- `CreateDeviceBuffer` accepts raw buffer data and buffer description, does not directly depend on `MeshResource` type
- 028-Texture module calls 030 via `EnsureDeviceResources` method, passing `GetPixelData()` etc. data parameters
- 012-Mesh module calls 030 via `EnsureDeviceResources` method, passing `GetVertexData()` etc. data parameters
- This design maintains module decoupling, per dependency graph requirements (030 does not depend on 028 or 012)

### Command List Pool and Staging Buffer Management

- Command list pools and staging buffers managed per `IDevice`, each device has independent pools
- Uses global map to store resource managers for each device
- Thread-safe, supports multi-threaded concurrent access

### Async Operations

- Async operations use command list pools and Fence synchronization
- Callbacks execute on agreed thread (default main thread)
- Each resource module (028-Texture, 012-Mesh) can handle batch optimization in their own `EnsureDeviceResources` implementation, calling 030's single resource creation interface

---

## Change Log

| Date | Change |
|------|--------|
| 2026-02-10 | Complete ABI table: DeviceResourceManager static methods, ResourceOperationStatus, ResourceOperationHandle, CommandListPool, StagingBufferManager |
| 2026-02-11 | Added internal implementation notes; clarified data-oriented interface design |
| 2026-02-22 | Updated dependencies: removed 013-Resource (030 uses data-oriented interface, no direct dependency); all symbols verified against actual implementation |
