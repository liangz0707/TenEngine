# 030-DeviceResourceManager 模块描述

## 1. 模块简要说明

DeviceResourceManager 提供**统一GPU资源管理器**：统一管理所有GPU资源类型（Texture、Mesh、Material等）的创建、上传、同步和生命周期，对应 Unreal 的 **RHI Resource Manager**、Unity 的 **Graphics Buffer/Texture Manager**。

**核心职责**：
- **统一管理所有GPU资源类型**：Texture、Mesh、Material等
- **命令列表管理**：命令列表池（用于异步上传），按IDevice管理，线程安全
- **数据上传**：暂存缓冲管理，支持同步和异步上传，按IDevice管理
- **资源屏障**：统一处理资源状态转换（CopyDst -> ShaderResource等）
- **同步机制**：Fence管理，支持异步操作完成回调
- **生命周期管理**：GPU资源与RResource绑定，延迟销毁

**不处理Shader/PSO**：
- Shader编译由010-Shader负责（CPU操作，不涉及GPU资源创建）
- PSO创建由Material或Pipeline直接调用008-RHI的CreateGraphicsPSO接口

## 2. 详细功能描述

### 2.1 Texture GPU资源创建

- **同步创建**：`CreateDeviceTexture(textureResource, device) -> ITexture*`
  - 从TextureResource获取像素数据和TextureDesc
  - 调用IDevice::CreateTexture创建GPU纹理
  - 使用暂存缓冲上传数据
  - 设置资源屏障（CopyDst -> ShaderResource）
  - 将ITexture*存储到TextureResource内部

- **异步创建**：`CreateDeviceTextureAsync(textureResource, device, callback, user_data)`
  - 使用命令列表池获取命令列表
  - 创建GPU纹理并上传数据（异步）
  - 创建Fence并注册回调
  - 回调在约定线程执行

### 2.2 Mesh GPU资源创建（后续阶段）

- **同步创建**：`CreateDeviceBuffer(meshResource, bufferType, device) -> IBuffer*`
  - 从MeshResource获取顶点/索引数据
  - 创建GPU缓冲并上传数据

- **异步创建**：`CreateDeviceBufferAsync(meshResource, bufferType, device, callback, user_data)`

### 2.3 批量操作

- **批量创建**：`CreateDeviceResourcesBatch(resources, count, device)`
  - 按资源类型分组（Texture、Mesh、Material）
  - 同一类型的资源合并到一个命令列表
  - 共享暂存缓冲（如果大小足够）

### 2.4 命令列表池

- 每个IDevice对应一个命令列表池
- 线程安全，支持多线程获取和归还
- 池大小可配置（默认8-16个命令列表）
- 优先复用，池空时创建新命令列表

### 2.5 暂存缓冲管理

- 每个IDevice对应一个暂存缓冲管理器
- 大小限制可配置（默认64MB）
- 支持多级缓冲（4MB、16MB、64MB）
- 优先使用现有缓冲，不足时创建新缓冲

### 2.6 清理机制

- `CleanupDevice(device)`：清理指定设备的命令列表池和暂存缓冲
- 调用时机：IDevice销毁前，由调用方手动调用
- 线程安全：需要保证调用时没有其他线程正在使用该设备

## 3. 实现难度

**中高**。需要处理命令列表池、暂存缓冲管理、资源屏障、Fence同步等复杂逻辑，需要与多个模块（028、012、011）集成。

## 4. 操作的资源类型

- **输入**：
  - TextureResource（ResourceType::Texture）：通过IResource接口获取像素数据和描述
  - MeshResource（ResourceType::Mesh）：通过IResource接口获取顶点/索引数据（后续阶段）
  - MaterialResource（ResourceType::Material）：通过IResource接口获取材质参数（后续阶段）

- **输出**：
  - ITexture*：GPU纹理句柄，存储在TextureResource内部
  - IBuffer*：GPU缓冲句柄，存储在MeshResource内部（后续阶段）

## 5. 是否有子模块

无。所有功能集中在DeviceResourceManager类中，CommandListPool和StagingBufferManager为内部实现类。

## 6. 模块上下游

### 6.1 和上下游交互、传递的数据类型

**上游依赖**：
- **001-Core**：内存分配、线程、日志、容器
- **008-RHI**：IDevice、ICommandList、ITexture、IBuffer、IFence、ISemaphore
- **013-Resource**：IResource接口、ResourceType枚举

**下游依赖**：
- **028-Texture**：使用CreateDeviceTexture创建GPU纹理
- **012-Mesh**：使用CreateDeviceBuffer创建GPU缓冲（后续阶段）
- **011-Material**：使用BindMaterialResources绑定GPU资源（后续阶段）

### 6.2 依赖关系图

```
001-Core ──┐
008-RHI ───┼──> 030-DeviceResourceManager
013-Resource┘

030-DeviceResourceManager
    ├──> 028-Texture
    ├──> 012-Mesh（后续阶段）
    └──> 011-Material（后续阶段）
```

## 7. 依赖关系

### 7.1 直接依赖

| 模块 | 依赖内容 | 用途说明 |
|------|----------|----------|
| **001-Core** | 内存分配、线程、日志、容器 | 内存管理、线程安全、日志记录 |
| **008-RHI** | IDevice、ICommandList、ITexture、IBuffer、IFence、ISemaphore | 创建GPU资源、命令列表管理、同步对象 |
| **013-Resource** | IResource接口、ResourceType枚举 | 识别资源类型、获取资源数据 |

### 7.2 被依赖关系

| 模块 | 依赖内容 | 用途说明 |
|------|----------|----------|
| **028-Texture** | CreateDeviceTexture | 创建GPU纹理资源 |
| **012-Mesh** | CreateDeviceBuffer | 创建GPU缓冲资源（后续阶段） |
| **011-Material** | BindMaterialResources | 绑定GPU资源（后续阶段） |

## 8. 与现有架构的集成

- **013-Resource**：各资源类型（TextureResource、MeshResource、MaterialResource）继承IResource
- **028-Texture**：EnsureDeviceResources调用030-DeviceResourceManager创建纹理
- **012-Mesh**：EnsureDeviceResources调用030-DeviceResourceManager创建缓冲（后续阶段）
- **011-Material**：EnsureDeviceResources调用030-DeviceResourceManager绑定资源（后续阶段）
- **019-PipelineCore**：PrepareRenderResources统一调用各资源的EnsureDeviceResources（可选，通过各资源模块间接使用）
- **008-RHI**：030-DeviceResourceManager调用IDevice接口创建GPU资源

## 9. 设计决策

### 9.1 静态方法 vs 单例

**选择**：静态方法
- 优点：简单，无需管理实例
- 命令列表池和暂存缓冲按IDevice管理（内部使用map<IDevice*, Pool>）

### 9.2 资源类型识别

**方法**：通过`IResource::GetResourceType()`识别资源类型，然后使用`dynamic_cast`转换
- 需要RTTI支持（C++默认启用）
- 先检查GetResourceType()，再转换，避免错误类型

### 9.3 命令列表池和暂存缓冲的清理

**方法**：提供`CleanupDevice(IDevice*)`手动清理接口
- 调用方在IDevice销毁前调用此接口
- 避免自动清理的复杂性和潜在的内存泄漏

### 9.4 Mipmap生成

**策略**：在CPU生成Mipmap（导入时或加载时）
- TextureResource::Load时，如果AssetDesc包含Mipmap数据，直接使用；否则在CPU生成
- 逐级上传每个Mipmap级别到GPU
- Mipmap生成由028-Texture模块负责，不纳入030

### 9.5 纹理压缩

**策略**：导入时压缩，运行时使用压缩纹理
- TextureResource::Import时，将源文件转换为压缩格式（如BC/DXT），保存为压缩纹理数据
- 运行时直接上传压缩纹理数据到GPU，无需解压

### 9.6 批量操作

**策略**：同一类型的资源合并到一个命令列表
- 按资源类型分组（Texture、Mesh、Material）
- 每组使用同一个命令列表，批量记录上传命令
- 减少命令列表切换开销，提高GPU利用率
