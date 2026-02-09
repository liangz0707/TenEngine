# 契约：030-DeviceResourceManager 模块对外 API

## 适用模块

- **实现方**：030-DeviceResourceManager（L2；统一GPU资源管理器，命令列表池、暂存缓冲管理、同步/异步GPU资源创建）
- **对应规格**：`docs/module-specs/030-device-resource-manager.md`
- **依赖**：001-Core、008-RHI、013-Resource

## 消费者

- **028-Texture**：创建GPU纹理资源（通过EnsureDeviceResources调用）
- **012-Mesh**：创建GPU缓冲资源（后续阶段）
- **011-Material**：绑定GPU资源（后续阶段）
- **019-PipelineCore**：PrepareRenderResources统一调用（可选，通过各资源模块间接使用）

---

## 模块定位与核心职责

030-DeviceResourceManager 提供**统一GPU资源管理器**，负责：
- **统一管理所有GPU资源类型**：Texture、Buffer等
- **命令列表管理**：命令列表池（用于异步上传），按IDevice管理
- **数据上传**：暂存缓冲管理，支持同步和异步上传
- **资源屏障**：统一处理资源状态转换
- **同步机制**：Fence管理，支持异步操作完成回调
- **生命周期管理**：GPU资源与RResource绑定，延迟销毁

030 **不处理Shader/PSO**：Shader编译由010-Shader负责（CPU操作），PSO创建由Material或Pipeline直接调用008-RHI。

030 **采用数据导向接口**：不直接依赖028-Texture或012-Mesh的具体类型，而是接受原始数据参数（pixelData、bufferData等），避免循环依赖。

---

## 能力列表

### 1. DeviceResourceManager 静态类

DeviceResourceManager 提供静态方法统一管理GPU资源创建，无需实例化。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **DeviceResourceManager** | GPU资源管理器（静态方法类） | 全局静态，无需实例化 |

#### 1.1 纹理资源创建

**同步创建GPU纹理**：
```cpp
static rhi::ITexture* CreateDeviceTexture(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device);
```
- **参数**：
  - `pixelData`：像素数据指针（RGBA8格式）
  - `pixelDataSize`：像素数据大小（字节）
  - `textureDesc`：RHI纹理描述（宽度、高度、格式等）
  - `device`：RHI设备
- **返回**：GPU纹理句柄，失败返回`nullptr`
- **说明**：同步创建GPU纹理并上传数据，设置资源屏障

**异步创建GPU纹理**：
```cpp
static bool CreateDeviceTextureAsync(
    void const* pixelData,
    size_t pixelDataSize,
    rhi::TextureDesc const& textureDesc,
    rhi::IDevice* device,
    void (*callback)(rhi::ITexture* texture, bool success, void* user_data),
    void* user_data);
```
- **参数**：
  - `pixelData`：像素数据指针
  - `pixelDataSize`：像素数据大小（字节）
  - `textureDesc`：RHI纹理描述
  - `device`：RHI设备
  - `callback`：完成回调函数
  - `user_data`：用户数据
- **返回**：`true`表示异步操作已启动，`false`表示启动失败
- **说明**：使用命令列表池异步创建GPU纹理，回调在约定线程执行

**更新GPU纹理数据**：
```cpp
static bool UpdateDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device,
    void const* data,
    size_t size);
```
- **参数**：
  - `texture`：GPU纹理句柄
  - `device`：RHI设备
  - `data`：源数据
  - `size`：数据大小（字节）
- **返回**：`true`表示成功，`false`表示失败

**销毁GPU纹理**：
```cpp
static void DestroyDeviceTexture(
    rhi::ITexture* texture,
    rhi::IDevice* device);
```
- **参数**：
  - `texture`：GPU纹理句柄
  - `device`：RHI设备
- **说明**：销毁GPU纹理资源

#### 1.2 缓冲资源创建

**同步创建GPU缓冲**：
```cpp
static rhi::IBuffer* CreateDeviceBuffer(
    resource::IResource* meshResource,
    rhi::BufferUsage bufferType,
    rhi::IDevice* device);
```
- **参数**：
  - `meshResource`：网格资源（必须是`ResourceType::Mesh`）
  - `bufferType`：缓冲类型（Vertex或Index）
  - `device`：RHI设备
- **返回**：GPU缓冲句柄，失败返回`nullptr`
- **说明**：从MeshResource创建GPU缓冲并上传数据

**异步创建GPU缓冲**：
```cpp
static bool CreateDeviceBufferAsync(
    resource::IResource* meshResource,
    rhi::BufferUsage bufferType,
    rhi::IDevice* device,
    void (*callback)(rhi::IBuffer* buffer, bool success, void* user_data),
    void* user_data);
```
- **参数**：
  - `meshResource`：网格资源
  - `bufferType`：缓冲类型
  - `device`：RHI设备
  - `callback`：完成回调函数
  - `user_data`：用户数据
- **返回**：`true`表示异步操作已启动，`false`表示启动失败

**销毁GPU缓冲**：
```cpp
static void DestroyDeviceBuffer(
    rhi::IBuffer* buffer,
    rhi::IDevice* device);
```
- **参数**：
  - `buffer`：GPU缓冲句柄
  - `device`：RHI设备
- **说明**：销毁GPU缓冲资源

#### 1.3 批量操作

**同步批量创建GPU资源**：
```cpp
static bool CreateDeviceResourcesBatch(
    resource::IResource** resources,
    size_t count,
    rhi::IDevice* device);
```
- **参数**：
  - `resources`：资源指针数组
  - `count`：资源数量
  - `device`：RHI设备
- **返回**：`true`表示所有资源创建成功，`false`表示部分或全部失败
- **说明**：按资源类型分组，同一类型合并到一个命令列表处理

**异步批量创建GPU资源**：
```cpp
static bool CreateDeviceResourcesBatchAsync(
    resource::IResource** resources,
    size_t count,
    rhi::IDevice* device,
    void (*callback)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data),
    void* user_data);
```
- **参数**：
  - `resources`：资源指针数组
  - `count`：资源数量
  - `device`：RHI设备
  - `callback`：完成回调函数，`success_flags`数组指示每个资源的创建结果
  - `user_data`：用户数据
- **返回**：`true`表示异步操作已启动，`false`表示启动失败

#### 1.4 清理操作

**清理设备资源**：
```cpp
static void CleanupDevice(rhi::IDevice* device);
```
- **参数**：
  - `device`：RHI设备
- **说明**：清理指定设备的命令列表池和暂存缓冲，应在IDevice销毁前调用

### 2. CommandListPool 类（公开接口）

CommandListPool 提供命令列表池管理，用于高效的异步GPU资源创建。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **CommandListPool** | 命令列表池（按IDevice管理） | 与IDevice绑定 |

**构造函数**：
```cpp
explicit CommandListPool(rhi::IDevice* device);
```

**获取命令列表**：
```cpp
rhi::ICommandList* Acquire();
```
- **返回**：命令列表指针，如果池为空则创建新的
- **说明**：线程安全

**释放命令列表**：
```cpp
void Release(rhi::ICommandList* cmd);
```
- **参数**：
  - `cmd`：命令列表指针
- **说明**：将命令列表返回到池中以便重用

**清空池**：
```cpp
void Clear();
```
- **说明**：清空所有命令列表

### 3. StagingBufferManager 类（公开接口）

StagingBufferManager 提供暂存缓冲管理，用于高效的GPU数据上传。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **StagingBufferManager** | 暂存缓冲管理器（按IDevice管理） | 与IDevice绑定 |

**构造函数**：
```cpp
explicit StagingBufferManager(rhi::IDevice* device);
```

**分配暂存缓冲**：
```cpp
rhi::IBuffer* Allocate(size_t size);
```
- **参数**：
  - `size`：请求的大小（字节）
- **返回**：暂存缓冲指针，分配失败返回`nullptr`
- **说明**：分配至少请求大小的暂存缓冲，线程安全

**释放暂存缓冲**：
```cpp
void Release(rhi::IBuffer* buffer);
```
- **参数**：
  - `buffer`：暂存缓冲指针
- **说明**：将暂存缓冲返回到池中

**清空所有缓冲**：
```cpp
void Clear();
```
- **说明**：清空所有暂存缓冲

---

## 类型与回调

### 回调类型

| 名称 | 类型定义 | 说明 |
|------|----------|------|
| **TextureCreateCallback** | `void (*)(rhi::ITexture* texture, bool success, void* user_data)` | 纹理创建完成回调 |
| **BufferCreateCallback** | `void (*)(rhi::IBuffer* buffer, bool success, void* user_data)` | 缓冲创建完成回调 |
| **BatchCreateCallback** | `void (*)(resource::IResource** resources, size_t count, bool* success_flags, void* user_data)` | 批量创建完成回调 |

---

## 命名空间

所有公开接口位于 `te::deviceresource` 命名空间。

---

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

---

## 约束

- 须在 Core、RHI、Resource 初始化之后使用。
- 调用方在IDevice销毁前必须调用`CleanupDevice`清理资源。
- 资源类型识别：通过`IResource::GetResourceType()`识别资源类型，然后使用`dynamic_cast`转换。
- 命令列表池和暂存缓冲按IDevice管理，线程安全。
- 异步操作的回调在约定线程执行（默认主线程，与013-Resource的LoadCompleteCallback一致）。
- **数据导向接口**：纹理创建接口接受原始数据参数，不直接依赖028-Texture的具体类型，避免循环依赖。

---

## 使用示例

### 同步创建纹理

```cpp
// 从TextureResource获取数据
void const* pixelData = textureResource->GetPixelData();
size_t pixelDataSize = textureResource->GetPixelDataSize();
rhi::TextureDesc textureDesc = textureResource->GetRHITextureDesc();

// 创建GPU纹理
rhi::ITexture* gpuTexture = te::deviceresource::DeviceResourceManager::CreateDeviceTexture(
    pixelData, pixelDataSize, textureDesc, device);

if (gpuTexture) {
    textureResource->SetDeviceTexture(gpuTexture);
}
```

### 异步创建纹理

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

### 清理设备资源

```cpp
// 在IDevice销毁前调用
te::deviceresource::DeviceResourceManager::CleanupDevice(device);
```
