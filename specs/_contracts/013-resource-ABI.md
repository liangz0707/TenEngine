# 013-Resource 模块 ABI

- **契约**：[013-resource-public-api.md](./013-resource-public-api.md)（能力与类型描述）
- **本文件**：013-Resource 对外 ABI 显式表。
- **统一加载接口**：**所有**资源类型（Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等）均通过 **同一** **RequestLoadAsync(path, type, ...)** 加载，无按类型拆开的多套 API。
- **命名**：成员方法采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。
- **资源三态**：**FResource**（硬盘形态，通过 **GUID** 引用其他资源，硬盘加载使用）；**RResource**（内存形态，根据 FResource 引用通过**指针**引用其他 RResource，内存引用使用）；**DResource**（GPU 形态，**直接保存在 RResource 内部**）。部分资源可能只存在某一形态。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 核心类型与枚举

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | — | 枚举 | 资源类型 | te/resource/ResourceTypes.h | ResourceType | `enum class ResourceType { Texture, Mesh, Material, Model, Effect, Terrain, Shader, Audio, Level, Custom, _Count };` 所有类型均经统一 RequestLoadAsync 加载；Level 供 029-World 关卡加载使用 |
| 013-Resource | te::resource | — | 枚举 | 加载状态 | te/resource/ResourceTypes.h | LoadStatus | `enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled };` 定义在 ResourceTypes.h 中 |
| 013-Resource | te::resource | — | 枚举 | 加载结果 | te/resource/ResourceTypes.h | LoadResult | `enum class LoadResult { Ok, NotFound, Error, Cancelled };` 定义在 ResourceTypes.h 中 |
| 013-Resource | te::resource | — | 枚举 | 加载优先级 | te/resource/ResourceTypes.h | LoadPriority | `enum class LoadPriority : int { Background = 0, Low = 25, Normal = 50, High = 75, Critical = 100 };` |
| 013-Resource | te::resource | — | 枚举 | 回调线程策略 | te/resource/ResourceTypes.h | CallbackThreadStrategy | `enum class CallbackThreadStrategy : int { MainThread, CallingThread, AnyWorker };` |
| 013-Resource | te::resource | — | 枚举 | 递归加载状态 | te/resource/ResourceTypes.h | RecursiveLoadState | `enum class RecursiveLoadState { NotLoaded, Loading, PartiallyReady, Ready, Failed, Cancelled };` |
| 013-Resource | te::resource | — | 枚举 | 资源状态事件 | te/resource/ResourceTypes.h | ResourceStateEvent | `enum class ResourceStateEvent { Created, Loading, Loaded, DependenciesReady, DeviceReady, Unloading, Unloaded, Reloaded, Error };` |
| 013-Resource | te::resource | — | 结构体 | 批量加载结果 | te/resource/ResourceTypes.h | BatchLoadResult | `struct BatchLoadResult { std::size_t totalCount; std::size_t successCount; std::size_t failedCount; std::size_t cancelledCount; LoadResult overallResult; };` |
| 013-Resource | te::resource | — | 结构体 | 加载请求信息 | te/resource/ResourceTypes.h | LoadRequestInfo | `struct LoadRequestInfo { const char* path; ResourceType type; LoadPriority priority; void* user_data; };` |
| 013-Resource | te::resource | — | 结构体 | 加载选项 | te/resource/ResourceManager.h | LoadOptions | `struct LoadOptions { LoadPriority priority; CallbackThreadStrategy callbackThread; bool preloadDependencies; void* user_data; };` |
| 013-Resource | te::resource | — | 类型别名 | 资源全局唯一 ID | te/resource/ResourceId.h | ResourceId | `using ResourceId = object::GUID;` 等价 GUID；FResource 间引用、可寻址路径、与 Object 引用解析对接 |
| 013-Resource | te::resource | — | 特化 | ResourceId 哈希 | te/resource/ResourceId.h | std::hash<ResourceId> | `template<> struct std::hash<te::resource::ResourceId>;` std::hash 特化，用于 unordered_map 等容器 |

### IResourceManager 接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | — | 类型别名/句柄 | 加载请求 ID | te/resource/ResourceManager.h | LoadRequestId | `using LoadRequestId = void*;` 不透明句柄，由 RequestLoadAsync 返回 |
| 013-Resource | te::resource | — | 类型别名/句柄 | 批量加载请求 ID | te/resource/ResourceManager.h | BatchLoadRequestId | `using BatchLoadRequestId = void*;` 不透明句柄，由 RequestLoadBatchAsync 返回 |
| 013-Resource | te::resource | — | 类型别名/句柄 | 流式句柄 | te/resource/ResourceManager.h | StreamingHandle | `using StreamingHandle = void*;` 不透明句柄，用于流式加载 |
| 013-Resource | te::resource | — | 类型别名/句柄 | 批量加载句柄 | te/resource/ResourceManager.h | BatchLoadHandle | `using BatchLoadHandle = void*;` 不透明句柄 |
| 013-Resource | te::resource | — | 回调类型 | 加载完成回调 | te/resource/ResourceManager.h | LoadCompleteCallback | `using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);` 在约定线程调用 |
| 013-Resource | te::resource | — | 回调类型 | 批量加载完成回调 | te/resource/ResourceManager.h | BatchLoadCompleteCallback | `using BatchLoadCompleteCallback = void (*)(BatchLoadHandle handle, BatchLoadResult const* result, void* user_data);` |
| 013-Resource | te::resource | — | 回调类型 | 资源状态回调 | te/resource/ResourceManager.h | ResourceStateCallback | `using ResourceStateCallback = void (*)(ResourceId id, ResourceStateEvent event, void* user_data);` |
| 013-Resource | te::resource | — | 类型别名 | 资源工厂函数 | te/resource/ResourceManager.h | ResourceFactory | `using ResourceFactory = IResource* (*)(ResourceType);` 资源工厂函数指针类型 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **统一**资源异步加载接口（线程安全） | te/resource/ResourceManager.h | IResourceManager::RequestLoadAsync | `LoadRequestId RequestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data);` 异步加载入口 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 带扩展选项的异步加载 | te/resource/ResourceManager.h | IResourceManager::RequestLoadAsyncEx | `LoadRequestId RequestLoadAsyncEx(char const* path, ResourceType type, LoadCompleteCallback on_done, LoadOptions const& options);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 批量异步加载 | te/resource/ResourceManager.h | IResourceManager::RequestLoadBatchAsync | `BatchLoadRequestId RequestLoadBatchAsync(LoadRequestInfo const* requests, std::size_t count, BatchLoadCompleteCallback on_done, void* user_data, LoadOptions const& options);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取批量加载结果 | te/resource/ResourceManager.h | IResourceManager::GetBatchLoadResult | `bool GetBatchLoadResult(BatchLoadRequestId id, BatchLoadResult& out_result) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载状态 | te/resource/ResourceManager.h | IResourceManager::GetLoadStatus | `LoadStatus GetLoadStatus(LoadRequestId id) const;` 线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载进度 | te/resource/ResourceManager.h | IResourceManager::GetLoadProgress | `float GetLoadProgress(LoadRequestId id) const;` 返回 0.f～1.f；线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 取消加载 | te/resource/ResourceManager.h | IResourceManager::CancelLoad | `void CancelLoad(LoadRequestId id);` 取消未完成的请求 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 取消批量加载 | te/resource/ResourceManager.h | IResourceManager::CancelBatchLoad | `void CancelBatchLoad(BatchLoadRequestId id);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 缓存查询 | te/resource/ResourceManager.h | IResourceManager::GetCached | `IResource* GetCached(ResourceId id) const;` 仅查缓存，未命中返回 nullptr |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 同步加载 | te/resource/ResourceManager.h | IResourceManager::LoadSync | `IResource* LoadSync(char const* path, ResourceType type);` 同步加载入口；阻塞直至完成 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 卸载 | te/resource/ResourceManager.h | IResourceManager::Unload | `void Unload(IResource* resource);` 递减引用计数 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 递归加载状态 | te/resource/ResourceManager.h | IResourceManager::GetRecursiveLoadState | `RecursiveLoadState GetRecursiveLoadState(ResourceId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 按请求 ID 获取递归状态 | te/resource/ResourceManager.h | IResourceManager::GetRecursiveLoadStateByRequestId | `RecursiveLoadState GetRecursiveLoadStateByRequestId(LoadRequestId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 检查资源就绪 | te/resource/ResourceManager.h | IResourceManager::IsResourceReady | `bool IsResourceReady(ResourceId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 按请求 ID 检查资源就绪 | te/resource/ResourceManager.h | IResourceManager::IsResourceReadyByRequestId | `bool IsResourceReadyByRequestId(LoadRequestId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 订阅资源状态 | te/resource/ResourceManager.h | IResourceManager::SubscribeResourceState | `void* SubscribeResourceState(ResourceId id, ResourceStateCallback callback, void* user_data);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 订阅全局资源状态 | te/resource/ResourceManager.h | IResourceManager::SubscribeGlobalResourceState | `void* SubscribeGlobalResourceState(ResourceStateCallback callback, void* user_data);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 取消订阅 | te/resource/ResourceManager.h | IResourceManager::UnsubscribeResourceState | `void UnsubscribeResourceState(void* subscription_handle);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 预加载依赖 | te/resource/ResourceManager.h | IResourceManager::PreloadDependencies | `LoadRequestId PreloadDependencies(ResourceId id, LoadCompleteCallback on_done, void* user_data);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取依赖树 | te/resource/ResourceManager.h | IResourceManager::GetDependencyTree | `bool GetDependencyTree(ResourceId id, std::vector<ResourceId>& out_dependencies, std::size_t max_depth = 0) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 流式请求 | te/resource/ResourceManager.h | IResourceManager::RequestStreaming | `StreamingHandle RequestStreaming(ResourceId id, int priority);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 设置流式优先级 | te/resource/ResourceManager.h | IResourceManager::SetStreamingPriority | `void SetStreamingPriority(StreamingHandle h, int priority);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 注册资源工厂 | te/resource/ResourceManager.h | IResourceManager::RegisterResourceFactory | `void RegisterResourceFactory(ResourceType type, ResourceFactory factory);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | Import | te/resource/ResourceManager.h | IResourceManager::Import | `bool Import(char const* path, ResourceType type, void* out_metadata_or_null);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | Save | te/resource/ResourceManager.h | IResourceManager::Save | `bool Save(IResource* resource, char const* path);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 寻址解析 | te/resource/ResourceManager.h | IResourceManager::ResolvePath | `char const* ResolvePath(ResourceId id) const;` GUID→路径 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 设置资源根目录 | te/resource/ResourceManager.h | IResourceManager::SetAssetRoot | `void SetAssetRoot(char const* path);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 加载所有清单 | te/resource/ResourceManager.h | IResourceManager::LoadAllManifests | `void LoadAllManifests();` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 解析资源类型 | te/resource/ResourceManager.h | IResourceManager::ResolveType | `ResourceType ResolveType(ResourceId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 按 GUID 同步加载 | te/resource/ResourceManager.h | IResourceManager::LoadSyncByGuid | `IResource* LoadSyncByGuid(ResourceId id);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 导入到仓库 | te/resource/ResourceManager.h | IResourceManager::ImportIntoRepository | `bool ImportIntoRepository(char const* sourcePath, ResourceType type, char const* repositoryName, char const* parentAssetPath, void* out_metadata_or_null);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 创建仓库 | te/resource/ResourceManager.h | IResourceManager::CreateRepository | `bool CreateRepository(char const* name);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取仓库列表 | te/resource/ResourceManager.h | IResourceManager::GetRepositoryList | `void GetRepositoryList(std::vector<std::string>& out) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 枚举所有资源 | te/resource/ResourceManager.h | IResourceManager::GetResourceInfos | `void GetResourceInfos(std::vector<ResourceInfo>& out) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取资源文件夹 | te/resource/ResourceManager.h | IResourceManager::GetAssetFolders | `void GetAssetFolders(std::vector<std::string>& out) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 移动资源到仓库 | te/resource/ResourceManager.h | IResourceManager::MoveResourceToRepository | `bool MoveResourceToRepository(ResourceId id, char const* targetRepository);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 更新资源路径 | te/resource/ResourceManager.h | IResourceManager::UpdateAssetPath | `bool UpdateAssetPath(ResourceId id, char const* newAssetPath);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 添加资源文件夹 | te/resource/ResourceManager.h | IResourceManager::AddAssetFolder | `bool AddAssetFolder(char const* repositoryName, char const* assetPath);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 移除资源文件夹 | te/resource/ResourceManager.h | IResourceManager::RemoveAssetFolder | `bool RemoveAssetFolder(char const* repositoryName, char const* assetPath);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取总内存使用 | te/resource/ResourceManager.h | IResourceManager::GetTotalMemoryUsage | `std::size_t GetTotalMemoryUsage() const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取资源内存使用 | te/resource/ResourceManager.h | IResourceManager::GetResourceMemoryUsage | `std::size_t GetResourceMemoryUsage(ResourceId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 设置内存预算 | te/resource/ResourceManager.h | IResourceManager::SetMemoryBudget | `void SetMemoryBudget(std::size_t budget_bytes);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 获取内存预算 | te/resource/ResourceManager.h | IResourceManager::GetMemoryBudget | `std::size_t GetMemoryBudget() const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 强制垃圾回收 | te/resource/ResourceManager.h | IResourceManager::ForceGarbageCollect | `std::size_t ForceGarbageCollect();` |
| 013-Resource | te::resource | — | 自由函数 | 获取全局 ResourceManager | te/resource/ResourceManager.h | GetResourceManager | `IResourceManager* GetResourceManager();` 由 Subsystems 注册或单例 |

### IResource 基类

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | IResource | 抽象基类 | 资源基类 | te/resource/Resource.h | IResource | 所有资源类型的基类；提供 Load、LoadAsync、Save、Import 方法及 protected 辅助方法；不直接构造；由 RequestLoadAsync 回调返回 |
| 013-Resource | te::resource | IResource | 抽象基类 | 查询资源类型 | te/resource/Resource.h | IResource::GetResourceType | `ResourceType GetResourceType() const;` 纯虚函数 |
| 013-Resource | te::resource | IResource | 抽象基类 | 获取资源 ID | te/resource/Resource.h | IResource::GetResourceId | `ResourceId GetResourceId() const;` 纯虚函数 |
| 013-Resource | te::resource | IResource | 抽象基类 | 释放资源 | te/resource/Resource.h | IResource::Release | `void Release();` 纯虚函数；递减引用计数 |
| 013-Resource | te::resource | IResource | 抽象基类 | 同步加载资源 | te/resource/Resource.h | IResource::Load | `bool Load(char const* path, IResourceManager* manager);` 虚函数，有默认实现 |
| 013-Resource | te::resource | IResource | 抽象基类 | 异步加载资源 | te/resource/Resource.h | IResource::LoadAsync | `bool LoadAsync(char const* path, IResourceManager* manager, LoadCompleteCallback on_done, void* user_data);` 虚函数，有默认实现 |
| 013-Resource | te::resource | IResource | 抽象基类 | 保存资源 | te/resource/Resource.h | IResource::Save | `bool Save(char const* path, IResourceManager* manager);` 虚函数，有默认实现 |
| 013-Resource | te::resource | IResource | 抽象基类 | 导入资源 | te/resource/Resource.h | IResource::Import | `bool Import(char const* sourcePath, IResourceManager* manager);` 虚函数，有默认实现 |
| 013-Resource | te::resource | IResource | 抽象基类 | 创建设备资源 | te/resource/Resource.h | IResource::EnsureDeviceResources | `void EnsureDeviceResources();` 虚函数，默认实现为空 |
| 013-Resource | te::resource | IResource | 抽象基类 | 查询设备资源是否就绪 | te/resource/Resource.h | IResource::IsDeviceReady | `virtual bool IsDeviceReady() const;` 默认返回 false |
| 013-Resource | te::resource | IResource | 抽象基类 | 异步创建设备资源 | te/resource/Resource.h | IResource::EnsureDeviceResourcesAsync | `void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data);` 虚函数，默认实现为空 |
| 013-Resource | te::resource | IResource | 保护模板方法 | 加载 AssetDesc | te/resource/Resource.h | IResource::LoadAssetDesc<T> | `template<typename T> std::unique_ptr<T> LoadAssetDesc(char const* path);` protected |
| 013-Resource | te::resource | IResource | 保护模板方法 | 保存 AssetDesc | te/resource/Resource.h | IResource::SaveAssetDesc<T> | `template<typename T> bool SaveAssetDesc(char const* path, T const* desc);` protected |
| 013-Resource | te::resource | IResource | 保护模板方法 | 加载依赖 | te/resource/Resource.h | IResource::LoadDependencies<T, GetDepsFn> | `template<typename T, typename GetDepsFn> bool LoadDependencies(T const* desc, GetDepsFn getDeps, IResourceManager* manager);` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 加载单个依赖 | te/resource/Resource.h | IResource::LoadDependency | `IResource* LoadDependency(ResourceId guid, IResourceManager* manager);` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 读取数据文件 | te/resource/Resource.h | IResource::LoadDataFile | `bool LoadDataFile(char const* path, void** outData, std::size_t* outSize);` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 写入数据文件 | te/resource/Resource.h | IResource::SaveDataFile | `bool SaveDataFile(char const* path, void const* data, std::size_t size);` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 生成 GUID | te/resource/Resource.h | IResource::GenerateGUID | `ResourceId GenerateGUID();` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 检测格式 | te/resource/Resource.h | IResource::DetectFormat | `std::string DetectFormat(char const* sourcePath);` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 获取 AssetDesc 路径 | te/resource/Resource.h | IResource::GetDescPath | `std::string GetDescPath(char const* path) const;` protected |
| 013-Resource | te::resource | IResource | 保护方法 | 获取数据文件路径 | te/resource/Resource.h | IResource::GetDataPath | `std::string GetDataPath(char const* path) const;` protected |
| 013-Resource | te::resource | IResource | 保护虚函数 | 加载完成钩子 | te/resource/Resource.h | IResource::OnLoadComplete | `virtual void OnLoadComplete();` protected |
| 013-Resource | te::resource | IResource | 保护虚函数 | 保存准备钩子 | te/resource/Resource.h | IResource::OnPrepareSave | `virtual void OnPrepareSave();` protected |
| 013-Resource | te::resource | IResource | 保护纯虚函数 | 转换源文件 | te/resource/Resource.h | IResource::OnConvertSourceFile | `virtual bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) = 0;` protected |
| 013-Resource | te::resource | IResource | 保护纯虚函数 | 创建 AssetDesc | te/resource/Resource.h | IResource::OnCreateAssetDesc | `virtual void* OnCreateAssetDesc() = 0;` protected |
| 013-Resource | te::resource | — | 类型特征 | AssetDesc 类型名 | te/resource/Resource.inl | AssetDescTypeName<T> | `template<typename T> struct AssetDescTypeName { static const char* Get(); };` 类型特征 |

### 资源视图接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | IShaderResource | 抽象接口 | Shader 资源视图 | te/resource/ShaderResource.h | IShaderResource | 继承 IResource；`void* GetShaderHandle() const = 0;` 实际类型为 te::shader::IShaderHandle*；由 010-Shader 实现 |
| 013-Resource | te::resource | ITextureResource | 抽象接口 | 纹理资源视图 | te/resource/TextureResource.h | ITextureResource | 继承 IResource；由 028-Texture 实现 |
| 013-Resource | te::resource | IMeshResource | 抽象接口 | 网格资源视图 | te/resource/MeshResource.h | IMeshResource | 继承 IResource；由 012-Mesh 实现 |
| 013-Resource | te::resource | — | 结构体 | 材质贴图槽位 | te/resource/MaterialResource.h | MaterialTextureSlot | `struct MaterialTextureSlot { std::uint32_t set; std::uint32_t binding; };` |
| 013-Resource | te::resource | IMaterialResource | 抽象接口 | 材质资源视图 | te/resource/MaterialResource.h | IMaterialResource | 继承 IResource；`GetTextureRefs(outSlots, outPaths, maxCount) const = 0;` 由 011-Material 实现 |
| 013-Resource | te::resource | IEffectResource | 抽象接口 | 特效资源视图 | te/resource/EffectResource.h | IEffectResource | 继承 IResource；粒子/VFX 等 |
| 013-Resource | te::resource | ITerrainResource | 抽象接口 | 地形资源视图 | te/resource/TerrainResource.h | ITerrainResource | 继承 IResource；地形块/高度图等 |

### 清单与仓库配置

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | — | 结构体 | 清单项 | te/resource/ResourceManifest.h | ManifestEntry | `struct ManifestEntry { ResourceId guid; std::string assetPath; ResourceType type; std::string repository; std::string displayName; };` |
| 013-Resource | te::resource | — | 结构体 | 资源清单 | te/resource/ResourceManifest.h | ResourceManifest | `struct ResourceManifest { std::vector<ManifestEntry> resources; std::vector<std::string> assetFolders; };` |
| 013-Resource | te::resource | — | 自由函数 | 加载清单 | te/resource/ResourceManifest.h | LoadManifest | `bool LoadManifest(char const* manifestPath, ResourceManifest& out);` |
| 013-Resource | te::resource | — | 自由函数 | 保存清单 | te/resource/ResourceManifest.h | SaveManifest | `bool SaveManifest(char const* manifestPath, ResourceManifest const& manifest);` |
| 013-Resource | te::resource | — | 自由函数 | 资源类型转字符串 | te/resource/ResourceManifest.h | ResourceTypeToString | `char const* ResourceTypeToString(ResourceType t);` |
| 013-Resource | te::resource | — | 自由函数 | 字符串转资源类型 | te/resource/ResourceManifest.h | ResourceTypeFromString | `ResourceType ResourceTypeFromString(char const* s);` |
| 013-Resource | te::resource | — | 结构体 | 仓库信息 | te/resource/ResourceRepositoryConfig.h | RepositoryInfo | `struct RepositoryInfo { std::string name; std::string root; std::string virtualPrefix; };` |
| 013-Resource | te::resource | — | 结构体 | 仓库配置 | te/resource/ResourceRepositoryConfig.h | RepositoryConfig | `struct RepositoryConfig { std::vector<RepositoryInfo> repositories; };` |
| 013-Resource | te::resource | — | 自由函数 | 加载仓库配置 | te/resource/ResourceRepositoryConfig.h | LoadRepositoryConfig | `bool LoadRepositoryConfig(char const* assetRoot, RepositoryConfig& out);` |
| 013-Resource | te::resource | — | 自由函数 | 保存仓库配置 | te/resource/ResourceRepositoryConfig.h | SaveRepositoryConfig | `bool SaveRepositoryConfig(char const* assetRoot, RepositoryConfig const& config);` |
| 013-Resource | te::resource | — | 自由函数 | 添加仓库 | te/resource/ResourceRepositoryConfig.h | AddRepository | `bool AddRepository(char const* assetRoot, char const* name);` |

### 扩展系统（详见各头文件）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | ResourceGroup | 类 | 资源组 | te/resource/ResourceGroup.h | ResourceGroup | AddResource、RemoveResource、LoadAllAsync、UnloadAll |
| 013-Resource | te::resource | IResourceGroupManager | 抽象接口 | 资源组管理器 | te/resource/ResourceGroup.h | IResourceGroupManager | CreateGroup、GetGroup、DestroyGroup |
| 013-Resource | te::resource | IResourceEventManager | 抽象接口 | 资源事件管理器 | te/resource/ResourceEvent.h | IResourceEventManager | SubscribeGlobal、SubscribeResource、BroadcastEvent |
| 013-Resource | te::resource | IHotReloadManager | 抽象接口 | 热重载管理器 | te/resource/ResourceHotReload.h | IHotReloadManager | SetConfig、ReloadResource、WatchAssetRoot |
| 013-Resource | te::resource | IStreamingManager | 抽象接口 | 流式加载管理器 | te/resource/ResourceStreaming.h | IStreamingManager | SetConfig、RegisterStreamable、ForceLOD、Update |
| 013-Resource | te::resource | IImportManager | 抽象接口 | 导入管理器 | te/resource/ResourceImport.h | IImportManager | RegisterPreset、ImportSync、ImportAsync、ImportBatchSync |
| 013-Resource | te::resource | IResourceTagManager | 抽象接口 | 资源标签管理器 | te/resource/ResourceTag.h | IResourceTagManager | CreateTag、AddTagToResource、GetResourcesWithTag |
| 013-Resource | te::resource | IResourceDebugManager | 抽象接口 | 资源调试管理器 | te/resource/ResourceDebug.h | IResourceDebugManager | GetProfiler、GetLeakDetector、DumpDebugInfo |
| 013-Resource | te::resource | IDownloadManager | 抽象接口 | 下载管理器 | te/resource/RemoteResource.h | IDownloadManager | QueueDownload、CancelDownload、PauseDownload |
| 013-Resource | te::resource | IChunkManager | 抽象接口 | Chunk/DLC 管理器 | te/resource/RemoteResource.h | IChunkManager | InstallChunk、UninstallChunk、GetAvailableDLCs |

*来源：用户故事 US-resource-001/002/003。契约能力：Import、Load、Unload、Streaming、Addressing（ResourceId/GUID）。*

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| 2026-02-10 | ResourceType 枚举增加 Level，供 029-World 关卡资源加载使用；IResource 增加 IsDeviceReady() 虚函数（默认 false），028/011 等重写，020 用于录制前过滤 |
| 2026-02-22 | 同步代码：新增 LoadPriority、CallbackThreadStrategy、RecursiveLoadState、ResourceStateEvent 枚举；新增 BatchLoadResult、LoadRequestInfo、LoadOptions 结构体；新增 IResourceManager 方法（RequestLoadAsyncEx、RequestLoadBatchAsync、GetBatchLoadResult、CancelBatchLoad、GetRecursiveLoadState、GetRecursiveLoadStateByRequestId、IsResourceReady、IsResourceReadyByRequestId、SubscribeResourceState、SubscribeGlobalResourceState、UnsubscribeResourceState、PreloadDependencies、GetDependencyTree、SetAssetRoot、LoadAllManifests、ResolveType、LoadSyncByGuid、ImportIntoRepository、CreateRepository、GetRepositoryList、GetResourceInfos、GetAssetFolders、GetAssetFoldersForRepository、MoveResourceToRepository、UpdateAssetPath、MoveAssetFolder、AddAssetFolder、RemoveAssetFolder、GetTotalMemoryUsage、GetResourceMemoryUsage、SetMemoryBudget、GetMemoryBudget、ForceGarbageCollect）；新增 ManifestEntry、ResourceManifest、RepositoryInfo、RepositoryConfig 结构体及相关函数；新增扩展系统（ResourceGroup、IResourceGroupManager、IResourceEventManager、IHotReloadManager、IStreamingManager、IImportManager、IResourceTagManager、IResourceDebugManager、IDownloadManager、IChunkManager） |
