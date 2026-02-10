# 契约：013-Resource 模块对外 API

## 适用模块

- **实现方**：013-Resource（L2；资源统一基类、文件加载、GUID 管理、序列化协调；不创建 DResource）
- **对应规格**：`docs/module-specs/013-resource.md`
- **依赖**：001-Core、002-Object

### IShaderResource 与 IMaterialResource（视图接口）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **IShaderResource** | Shader 资源视图；继承 IResource；`void* GetShaderHandle() const = 0;` 返回 te::shader::IShaderHandle*（调用方转型）；由 010-Shader 实现；经 LoadSync(..., ResourceType::Shader) 或 GetCached(shaderGuid) 返回 | 与 IResource 一致 |
| **IMaterialResource** | 材质资源视图；继承 IResource；持 Shader 引用、贴图 (set,binding)→引用、参数缓冲；`GetTextureRefs(outSlots, outPaths, maxCount)` 返回槽位与贴图 GUID 字符串；由 011-Material 实现 | 与 IResource 一致 |
| **MaterialTextureSlot** | 贴图槽位；`struct { std::uint32_t set; std::uint32_t binding; };` 与 GetTextureRefs 配合使用 | 调用方栈或数组 |

## 消费者

- 010-Shader、011-Material、012-Mesh、028-Texture、029-World（资源类型继承 IResource，实现 Load/Save/Import 方法，向 013 注册资源工厂）、016-Audio、020-Pipeline、022-2D、023-Terrain、024-Editor（调用 ResourceManager Load/缓存 等获取资源）

---

## 模块定位与核心职责

013-Resource 提供**资源统一基类 IResource**，所有资源类型继承该基类。IResource 基类实现通用的文件加载、GUID 管理、序列化调用、依赖解析等逻辑，各资源类型实现具体的 Load、Save、Import 方法。

ResourceManager 简化为协调器和缓存管理器，负责协调加载流程、缓存管理、异步调度、GUID 解析。

013 **不创建、不持有、不调用** 008-RHI；Load 阶段仅创建 RResource（内存形态），DResource（GPU 形态）由 011/012/028 在 EnsureDeviceResources 时创建。

---

## 能力列表

### 1. IResource 基类

IResource 是所有可加载资源的统一基类。各资源类型（Mesh、Texture、Material、Model、Shader 等）继承 IResource，实现具体的加载、保存、导入逻辑。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **IResource** | 可加载资源的统一基类；提供通用的文件加载、GUID 管理、序列化调用、依赖解析等逻辑；各资源类型继承并实现 Load、Save、Import 方法 | 013 缓存或下游经 ResourceId/句柄解析持有 |
| **ResourceType** | 资源类型枚举（Texture、Mesh、Material、Model、Effect、Terrain、Shader、Audio、**Level**、Custom 等）；用于请求加载与 IResource::GetResourceType()；Level 供 029-World 关卡加载 | 与类型绑定 |
| **ResourceId / GUID** | 资源全局唯一标识；缓存键、FResource 间引用、可寻址路径、与 Object 引用解析对接；`using ResourceId = object::GUID;` | 与资源绑定 |

#### IResource 核心方法

**必须实现的纯虚方法**：
- `GetResourceType() -> ResourceType`：返回资源类型
- `GetResourceId() -> ResourceId`：返回资源 GUID
- `Release()`：释放资源引用计数
- `OnConvertSourceFile(sourcePath, outData, outSize) -> bool`：转换源文件为引擎格式（protected 纯虚函数）
- `OnCreateAssetDesc() -> void*`：创建 AssetDesc 实例（protected 纯虚函数）

**有默认实现的虚方法**（子类可选择性重写）：
- `Load(path, manager) -> bool`：同步加载资源；默认实现仅验证输入并调用 OnLoadComplete()；子类应重写以调用 LoadAssetDesc<T>、LoadDataFile、LoadDependencies<T> 等
- `LoadAsync(path, manager, on_done, user_data) -> bool`：异步加载资源；默认实现使用 001-Core IThreadPool 在后台线程执行 Load，在约定线程调用回调
- `Save(path, manager) -> bool`：保存资源；默认实现调用 OnPrepareSave()；子类应重写以调用 SaveAssetDesc<T>、SaveDataFile 等
- `Import(sourcePath, manager) -> bool`：导入资源；默认实现调用 DetectFormat、OnConvertSourceFile、OnCreateAssetDesc、GenerateGUID、SaveAssetDesc、SaveDataFile
- `EnsureDeviceResources()`：创建 GPU 资源（DResource）；默认实现为空
- `EnsureDeviceResourcesAsync(on_done, user_data)`：异步创建 GPU 资源；默认实现为空

**保护虚拟钩子方法**（子类可选择性重写）：
- `OnLoadComplete()`：加载完成后的钩子；默认实现为空；子类可重写以执行资源特定初始化
- `OnPrepareSave()`：保存前的准备钩子；默认实现为空；子类可重写以准备保存数据

#### IResource 基类提供的辅助方法（protected）

IResource 基类提供以下 protected 辅助方法，供子类在 Load/Save/Import 实现中调用：

**模板方法**：
- `LoadAssetDesc<T>(path) -> std::unique_ptr<T>`：读取 AssetDesc 文件并反序列化（调用 002-Object）；需要 AssetDescTypeName<T> 特化
- `SaveAssetDesc<T>(path, desc) -> bool`：序列化 AssetDesc 并写入文件（调用 002-Object）；需要 AssetDescTypeName<T> 特化
- `LoadDependencies<T, GetDepsFn>(desc, getDeps, manager) -> bool`：从 AssetDesc 提取依赖列表并加载；自动选择同步/异步模式（根据 m_isLoadingAsync 标志）

**普通方法**：
- `LoadDataFile(path, outData, outSize) -> bool`：读取数据文件（调用 001-Core FileRead）；调用方必须释放 outData（使用 te::core::Free）
- `SaveDataFile(path, data, size) -> bool`：写入数据文件（调用 001-Core FileWrite）
- `LoadDependency(guid, manager) -> IResource*`：加载单个依赖资源（GUID → ResourceId，递归加载，同步模式）
- `GenerateGUID() -> ResourceId`：生成 GUID（调用 002-Object GUID::Generate）
- `DetectFormat(sourcePath) -> std::string`：检测源文件格式（通过文件扩展名，调用 001-Core PathGetExtension）
- `GetDescPath(path) -> std::string`：从资源路径生成 AssetDesc 文件路径
- `GetDataPath(path) -> std::string`：从资源路径生成数据文件路径（添加 .data 扩展）

**类型特征**：
- `AssetDescTypeName<T>`：类型特征，用于获取 AssetDesc 类型的字符串名称；资源模块应为各自的 AssetDesc 类型特化此特征；定义在 Resource.inl 中

### 2. IResourceManager 协调器

ResourceManager 简化为协调器和缓存管理器，负责协调加载流程、缓存管理、异步调度、GUID 解析。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **IResourceManager** | 统一加载入口与协调器；RequestLoadAsync、LoadSync、GetCached、Unload、GetLoadStatus、GetLoadProgress、CancelLoad、RequestStreaming、SetStreamingPriority、RegisterResourceFactory；调用 IResource::Load（同步）或 IResource::LoadAsync（异步）、Save、Import | 由 Subsystems 或单例提供，调用方不拥有指针 |
| **LoadRequestId** | 异步加载请求句柄；`using LoadRequestId = void*;` 由 RequestLoadAsync 返回 | 请求发出至完成或取消 |
| **LoadStatus / LoadResult** | 加载状态与结果；定义在 ResourceTypes.h 中；LoadStatus: Pending/Loading/Completed/Failed/Cancelled；LoadResult: Ok/NotFound/Error/Cancelled；供 GetLoadStatus、回调使用 | 与请求或回调绑定 |
| **LoadCompleteCallback** | 异步加载完成回调；`using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);` 在约定线程调用（由 IThreadPool::SetCallbackThread 指定，默认主线程） | 由调用方或框架管理 |
| **ResourceFactory** | 资源工厂函数指针；`using ResourceFactory = IResource* (*)(ResourceType);` 用于创建 IResource 实例 | 注册后持续有效 |
| **StreamingHandle** | 流式请求句柄；`using StreamingHandle = void*;` 用于流式加载 | 请求有效期内 |

#### ResourceManager 核心方法

- `RequestLoadAsync(path, type, callback, user_data) -> LoadRequestId`：异步加载资源；创建资源实例（优先使用 002-Object TypeRegistry::CreateInstance，回退到 ResourceFactory）并调用 IResource::LoadAsync；线程安全
- `LoadSync(path, type) -> IResource*`：同步加载资源；创建资源实例并调用 IResource::Load；阻塞直至完成；失败返回 nullptr；线程安全
- `GetCached(id) -> IResource*`：查询缓存；仅查缓存，未命中返回 nullptr，不触发加载；线程安全
- `Unload(resource)`：卸载资源；递减引用计数，当为零时从缓存移除；调用 IResource::Release；线程安全
- `GetLoadStatus(id) -> LoadStatus`：查询加载状态；线程安全
- `GetLoadProgress(id) -> float`：查询加载进度（0.0 到 1.0）；线程安全
- `CancelLoad(id)`：取消加载；取消未完成的请求；回调仍会触发，result 为 Cancelled；线程安全
- `RequestStreaming(id, priority) -> StreamingHandle`：请求流式加载；当前为占位实现
- `SetStreamingPriority(handle, priority)`：设置流式优先级；当前为占位实现
- `RegisterResourceFactory(type, factory)`：注册资源工厂；实现采用混合机制：优先使用 002-Object TypeRegistry，回退到 ResourceFactory
- `Import(path, type, out_metadata) -> bool`：导入资源；创建资源实例并调用 IResource::Import
- `Save(resource, path) -> bool`：保存资源；调用 IResource::Save
- `ResolvePath(id) -> char const*`：解析 ResourceId 到路径；GUID→路径；未解析返回 nullptr；线程安全

### 3. 资源缓存

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **GetCached(ResourceId)** | 按 ResourceId 查询已加载资源，返回 IResource*；命中则直接返回，未命中返回 nullptr（不触发加载） | 返回的 IResource* 与缓存一致 |
| **缓存策略** | 缓存存 IResource*，按 ResourceId 查找；使用引用计数（std::atomic<int>）；Unload/Release/GC 与引用计数协调；与各模块句柄释放顺序一致 | 与 013 实现约定 |

### 4. 资源三态（概念，用于理解数据流）

| 名称 | 语义 | 说明 |
|------|------|------|
| **FResource** | 硬盘形态 | 由 AssetDesc 文件和数据文件组成；引用其它资源仅通过 GUID；加载时从磁盘/包读入 |
| **RResource** | 运行时/内存形态 | 即实现 IResource 的对象；013 仅创建 RResource，不创建 DResource；DResource 槽位由 011/012/028/008 在 EnsureDeviceResources 时填充 |
| **DResource** | GPU 形态 | 由 008/011/012/028 在 EnsureDeviceResources 时创建，对 013 不可见，由 RResource 内部持有 |

### 5. 资源加载/保存/导入流程

#### 5.1 Load 流程

**同步加载（LoadSync）**：
```
ResourceManager::LoadSync
    │
    ├─> 检查缓存（GetCached）
    │   └─> 命中：直接返回
    │
    ├─> 未命中：创建资源实例（混合机制：优先 TypeRegistry::CreateInstance，回退到 ResourceFactory）
    │   └─> 调用 IResource::Load(path, manager)
    │       │
    │       ├─> 子类重写 Load()，调用基类辅助方法：
    │       │   ├─> LoadAssetDesc<T>(path) - 读取并反序列化 AssetDesc（002-Object）
    │       │   ├─> LoadDataFile(path, outData, outSize) - 读取数据文件（001-Core）
    │       │   ├─> LoadDependencies<T>(desc, getDeps, manager) - 加载依赖资源（递归，同步模式）
    │       │   └─> OnLoadComplete() - 资源特定初始化（子类可重写）
    │       │
    │       └─> 返回 IResource*
    │
    └─> 缓存 IResource*（按 ResourceId，引用计数初始化为 1）
```

**异步加载（RequestLoadAsync）**：
```
ResourceManager::RequestLoadAsync
    │
    ├─> 检查缓存（GetCached）
    │   └─> 命中：立即调用回调并返回
    │
    ├─> 未命中：创建资源实例（混合机制：优先 TypeRegistry::CreateInstance，回退到 ResourceFactory）
    │   └─> 调用 IResource::LoadAsync(path, manager, on_done, user_data)
    │       │
    │       ├─> 默认实现：使用 IThreadPool 在后台线程执行 Load
    │       │   ├─> 设置 m_isLoadingAsync = true
    │       │   ├─> 在后台线程调用 Load(path, manager)
    │       │   │   └─> 子类重写的 Load() 中，LoadDependencies<T> 检测到异步上下文，使用异步模式
    │       │   └─> 在约定线程调用回调（由 IThreadPool::SetCallbackThread 指定，默认主线程）
    │       │
    │       └─> 返回 LoadRequestId（用于状态跟踪）
    │
    └─> 缓存 IResource*（按 ResourceId，引用计数初始化为 1）
```

#### 5.2 Save 流程

```
ResourceManager::Save(IResource*, path)
    │
    └─> 调用 IResource::Save(path, manager)
        │
        ├─> 默认实现：调用 OnPrepareSave()（子类可重写）
        │
        └─> 子类重写 Save()，调用基类辅助方法：
            ├─> OnPrepareSave() - 准备保存数据（子类可重写）
            ├─> GenerateGUID() - 生成 GUID（002-Object）
            ├─> SaveAssetDesc<T>(path, desc) - 序列化并保存 AssetDesc（002-Object）
            └─> SaveDataFile(path, data, size) - 保存数据文件（001-Core）
```

#### 5.3 Import 流程

```
ResourceManager::Import(path, type)
    │
    └─> 创建资源实例（混合机制：优先 TypeRegistry::CreateInstance，回退到 ResourceFactory）
        │
        └─> 调用 IResource::Import(sourcePath, manager)
            │
            ├─> 默认实现：
            │   ├─> DetectFormat(sourcePath) - 检测格式（001-Core PathGetExtension）
            │   ├─> OnConvertSourceFile(sourcePath, outData, outSize) - 转换源文件（子类必须实现）
            │   ├─> OnCreateAssetDesc() - 创建 AssetDesc（子类必须实现）
            │   ├─> GenerateGUID() - 生成 GUID（002-Object）
            │   ├─> SaveAssetDesc<T>(path, desc) - 保存 AssetDesc（002-Object）
            │   └─> SaveDataFile(path, data, size) - 保存数据文件（001-Core）
            │
            └─> 返回成功/失败
```

### 6. 各资源类型的实现要求

**所有资源类型必须**：
1. **继承 IResource 基类**
2. **实现纯虚函数**：
   - `GetResourceType() -> ResourceType`
   - `GetResourceId() -> ResourceId`
   - `Release()`
   - `OnConvertSourceFile(sourcePath, outData, outSize) -> bool`（protected）
   - `OnCreateAssetDesc() -> void*`（protected）
3. **重写 Load 方法**（同步加载）：
   - 调用基类的 `LoadAssetDesc<T>` 读取并反序列化 AssetDesc（需要 AssetDescTypeName<T> 特化）
   - 调用基类的 `LoadDataFile` 读取数据文件
   - 调用基类的 `LoadDependencies<T>` 加载依赖资源（传入函数对象提取依赖列表）
   - 调用基类的 `OnLoadComplete()` 或重写此钩子以执行资源特定初始化
4. **可选重写 LoadAsync 方法**（异步加载）：
   - 默认实现使用 IThreadPool 在后台线程执行 Load，在约定线程调用回调
   - 如需自定义异步逻辑，可重写此方法
5. **重写 Save 方法**：
   - 调用基类的 `OnPrepareSave()` 或重写此钩子以准备保存数据
   - 调用基类的 `GenerateGUID` 生成 GUID（如果需要）
   - 调用基类的 `SaveAssetDesc<T>` 保存 AssetDesc 文件（需要 AssetDescTypeName<T> 特化）
   - 调用基类的 `SaveDataFile` 保存数据文件
6. **可选重写 Import 方法**：
   - 默认实现调用 DetectFormat、OnConvertSourceFile、OnCreateAssetDesc、GenerateGUID、SaveAssetDesc、SaveDataFile
   - 如需自定义导入逻辑，可重写此方法
7. **特化 AssetDescTypeName<T>**：
   - 为各自的 AssetDesc 类型特化 `AssetDescTypeName<T>` 类型特征，提供类型名称字符串

**AssetDesc 归属**：
- 各资源类型拥有自己的 AssetDesc（如 MeshAssetDesc→012-Mesh，TextureAssetDesc→028-Texture）
- AssetDesc 通过 002-Object 注册类型，支持序列化/反序列化
- IResource::Load/Save 内部调用 002-Object 的序列化接口

### 7. 资源工厂机制

**混合机制**：
- **优先**：使用 002-Object 的 `TypeRegistry::CreateInstance(typeName)` 创建资源实例
- **回退**：如果 TypeRegistry 中未注册，使用注册的 `ResourceFactory` 函数指针
- 资源类型模块应在初始化时：
  1. 通过 002-Object 注册资源类型（推荐）
  2. 或调用 `RegisterResourceFactory` 注册工厂函数

### 8. 其他跨边界类型（按需）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| StreamingHandle | 流式请求句柄；与 LOD/地形按需加载对接；当前为占位实现 | 请求有效期内 |
| Metadata | 资源元数据；格式、依赖记录、与导入管线对接 | 与资源或导入产物绑定 |

### 能力汇总（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | **IResource 基类** | 提供 IResource 基类，包含 Load、LoadAsync、Save、Import 方法及辅助方法；ResourceType 枚举；各资源类型继承并实现 |
| 2 | **统一加载** | RequestLoadAsync、LoadSync 为唯一入口；ResourceManager 协调，调用 IResource::Load/LoadAsync；Load 阶段不创建 DResource |
| 3 | **资源缓存** | 按 ResourceId 缓存 IResource*；GetCached(ResourceId)；使用引用计数；与 Unload/GC 协调 |
| 4 | **加载工具** | GetLoadStatus、GetLoadProgress、CancelLoad；RequestStreaming、SetStreamingPriority（占位实现） |
| 5 | **寻址** | ResourceId、GUID、可寻址路径、BundleMapping；GUID→路径解析（ResolvePath）；ResourceId 提供 std::hash 特化 |
| 6 | **卸载** | Unload(IResource*)、IResource::Release()；与各模块句柄协调；引用计数管理 |
| 7 | **EnsureDeviceResources** | EnsureDeviceResourcesAsync/EnsureDeviceResources 由下游触发并转发给具体 IResource 实现；013 不参与 DResource 创建 |
| 8 | **文件加载与 GUID 管理** | IResource 基类提供文件读取、GUID 解析、路径管理；调用 001-Core 文件 I/O、002-Object GUID 管理 |
| 9 | **序列化协调** | IResource 基类调用 002-Object 序列化/反序列化接口；各资源类型通过 AssetDesc 实现序列化；需要 AssetDescTypeName<T> 特化 |
| 10 | **模板方法模式** | IResource 基类提供默认实现（模板方法模式），子类可选择性重写特定步骤；Load/Save/Import 有默认实现，子类可重写以调用模板辅助方法 |
| 11 | **异步加载基础设施** | LoadAsync 默认实现使用 001-Core IThreadPool；回调在约定线程调用（由 SetCallbackThread 指定，默认主线程） |
| 12 | **资源工厂混合机制** | 优先使用 002-Object TypeRegistry，回退到 ResourceFactory 函数指针 |

*Desc 归属：ModelAssetDesc、IModelResource→029-World；TextureAssetDesc→028-Texture；ShaderAssetDesc→010，MaterialAssetDesc→011，LevelAssetDesc/SceneNodeDesc→029，MeshAssetDesc→012。各资源类型拥有自己的 AssetDesc，通过 002-Object 注册和序列化。*

---

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ResourceType 枚举增加 Level，供 029-World 关卡资源加载使用 |

---

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

---

## 约束

- 须在 Core、Object 初始化之后使用；各资源类型须继承 IResource 并实现 Load/Save/Import 方法。
- 013 不创建、不持有、不调用 008；资源引用格式须与 Object 序列化约定一致；句柄释放顺序须与卸载策略协调。
- IResource 基类提供通用逻辑（文件加载、GUID 管理、序列化调用），各资源类型实现具体逻辑。
- Load/Save/Import 有默认实现，但子类通常需要重写以调用模板辅助方法（LoadAssetDesc<T>、SaveAssetDesc<T> 等）。
- 资源类型模块必须为各自的 AssetDesc 类型特化 AssetDescTypeName<T> 类型特征。
