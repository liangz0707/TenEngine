# 030-DeviceResourceManager 模块 ABI

- **契约**：[030-device-resource-manager-public-api.md](./030-device-resource-manager-public-api.md)（能力与类型描述）
- **本文件**：030-DeviceResourceManager 对外 ABI 显式表。
- **命名**：成员方法采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 类 | GPU资源管理器（静态方法） | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager | 提供静态方法统一管理GPU资源创建（Texture、Buffer等） |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 同步创建GPU纹理 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceTexture | `static rhi::ITexture* CreateDeviceTexture(void const* pixelData, size_t pixelDataSize, rhi::TextureDesc const& textureDesc, rhi::IDevice* device);` 从像素数据创建GPU纹理，返回ITexture*，失败返回nullptr |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 异步创建GPU纹理 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceTextureAsync | `static bool CreateDeviceTextureAsync(void const* pixelData, size_t pixelDataSize, rhi::TextureDesc const& textureDesc, rhi::IDevice* device, void (*callback)(rhi::ITexture* texture, bool success, void* user_data), void* user_data);` 异步创建GPU纹理，回调在约定线程执行 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 更新GPU纹理数据 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::UpdateDeviceTexture | `static bool UpdateDeviceTexture(rhi::ITexture* texture, rhi::IDevice* device, void const* data, size_t size);` 更新GPU纹理数据 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 销毁GPU纹理 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::DestroyDeviceTexture | `static void DestroyDeviceTexture(rhi::ITexture* texture, rhi::IDevice* device);` 销毁GPU纹理资源 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 同步创建GPU缓冲 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceBuffer | `static rhi::IBuffer* CreateDeviceBuffer(resource::IResource* meshResource, rhi::BufferUsage bufferType, rhi::IDevice* device);` 从MeshResource创建GPU缓冲（顶点/索引），返回IBuffer*，失败返回nullptr |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 异步创建GPU缓冲 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceBufferAsync | `static bool CreateDeviceBufferAsync(resource::IResource* meshResource, rhi::BufferUsage bufferType, rhi::IDevice* device, void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data), void* user_data);` 异步创建GPU缓冲，回调在约定线程执行 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 销毁GPU缓冲 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::DestroyDeviceBuffer | `static void DestroyDeviceBuffer(rhi::IBuffer* buffer, rhi::IDevice* device);` 销毁GPU缓冲资源 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 批量创建GPU资源 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceResourcesBatch | `static bool CreateDeviceResourcesBatch(resource::IResource** resources, size_t count, rhi::IDevice* device);` 批量创建GPU资源，按类型分组，同一类型合并到一个命令列表 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 异步批量创建GPU资源 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CreateDeviceResourcesBatchAsync | `static bool CreateDeviceResourcesBatchAsync(resource::IResource** resources, size_t count, rhi::IDevice* device, void (*callback)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data), void* user_data);` 异步批量创建GPU资源，通过success_flags数组传递每个资源的创建结果 |
| 030-DeviceResourceManager | te::deviceresource | DeviceResourceManager | 静态方法 | 清理设备资源 | te/deviceresource/DeviceResourceManager.h | DeviceResourceManager::CleanupDevice | `static void CleanupDevice(rhi::IDevice* device);` 清理指定设备的命令列表池和暂存缓冲，应在IDevice销毁前调用 |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | 类 | 命令列表池 | te/deviceresource/CommandListPool.h | CommandListPool | 命令列表池管理，用于高效的异步GPU资源创建，按IDevice管理，线程安全 |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | 构造函数 | 创建命令列表池 | te/deviceresource/CommandListPool.h | CommandListPool::CommandListPool | `explicit CommandListPool(rhi::IDevice* device);` 创建命令列表池 |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | 方法 | 获取命令列表 | te/deviceresource/CommandListPool.h | CommandListPool::Acquire | `rhi::ICommandList* Acquire();` 从池中获取命令列表，如果池为空则创建新的，线程安全 |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | 方法 | 释放命令列表 | te/deviceresource/CommandListPool.h | CommandListPool::Release | `void Release(rhi::ICommandList* cmd);` 将命令列表返回到池中以便重用 |
| 030-DeviceResourceManager | te::deviceresource | CommandListPool | 方法 | 清空池 | te/deviceresource/CommandListPool.h | CommandListPool::Clear | `void Clear();` 清空所有命令列表 |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | 类 | 暂存缓冲管理器 | te/deviceresource/StagingBufferManager.h | StagingBufferManager | 暂存缓冲管理，用于高效的GPU数据上传，按IDevice管理，线程安全 |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | 构造函数 | 创建暂存缓冲管理器 | te/deviceresource/StagingBufferManager.h | StagingBufferManager::StagingBufferManager | `explicit StagingBufferManager(rhi::IDevice* device);` 创建暂存缓冲管理器 |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | 方法 | 分配暂存缓冲 | te/deviceresource/StagingBufferManager.h | StagingBufferManager::Allocate | `rhi::IBuffer* Allocate(size_t size);` 分配至少请求大小的暂存缓冲，分配失败返回nullptr，线程安全 |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | 方法 | 释放暂存缓冲 | te/deviceresource/StagingBufferManager.h | StagingBufferManager::Release | `void Release(rhi::IBuffer* buffer);` 将暂存缓冲返回到池中 |
| 030-DeviceResourceManager | te::deviceresource | StagingBufferManager | 方法 | 清空所有缓冲 | te/deviceresource/StagingBufferManager.h | StagingBufferManager::Clear | `void Clear();` 清空所有暂存缓冲 |

---

## 内部实现（不对外暴露ABI）

以下类为内部实现，不对外暴露ABI：

| 模块名 | 命名空间 | 类名 | 说明 |
|--------|----------|------|------|
| 030-DeviceResourceManager | te::deviceresource::internal | TextureDeviceImpl | 纹理GPU资源创建实现（内部实现） |
| 030-DeviceResourceManager | te::deviceresource::internal | ResourceUploadHelper | 资源上传辅助类（内部实现） |
| 030-DeviceResourceManager | te::deviceresource::internal | DeviceResources | 设备资源管理结构（内部实现） |
| 030-DeviceResourceManager | te::deviceresource::internal | AsyncOperationContext | 异步操作上下文（内部实现） |

---

## 依赖关系

### 上游依赖

| 模块 | 依赖内容 | 用途 |
|------|----------|------|
| **001-Core** | 日志、内存分配 | 日志记录、内存管理 |
| **008-RHI** | IDevice、ITexture、IBuffer、ICommandList、IFence、IQueue、ResourceState、TextureDesc、BufferUsage | GPU资源创建、命令列表、同步 |
| **013-Resource** | IResource、ResourceType | 资源类型识别、批量操作 |

### 下游依赖

| 模块 | 使用内容 | 用途 |
|------|----------|------|
| **028-Texture** | CreateDeviceTexture、CreateDeviceTextureAsync | 创建GPU纹理资源 |
| **012-Mesh** | CreateDeviceBuffer、CreateDeviceBufferAsync | 创建GPU缓冲资源（后续阶段） |

---

## 设计说明

### 数据导向接口

030模块采用数据导向接口设计，避免循环依赖：
- `CreateDeviceTexture`接受原始像素数据和纹理描述，不直接依赖`TextureResource`类型
- 028-Texture模块通过`EnsureDeviceResources`方法调用030，传入数据参数
- 这种设计保持了模块间的解耦，符合依赖关系图的要求

### 命令列表池和暂存缓冲管理

- 命令列表池和暂存缓冲按`IDevice`管理，每个设备有独立的池
- 使用全局映射表存储每个设备的资源管理器
- 线程安全，支持多线程并发访问

### 异步操作

- 异步操作使用命令列表池和Fence同步
- 回调在约定线程执行（默认主线程）
- 支持批量异步操作，通过`success_flags`数组传递每个资源的创建结果
