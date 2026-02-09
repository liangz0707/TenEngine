# 013-Resource 模块 ABI

- **契约**：[013-resource-public-api.md](./013-resource-public-api.md)（能力与类型描述）
- **本文件**：013-Resource 对外 ABI 显式表。
- **统一加载接口**：**所有**资源类型（Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等）均通过 **同一** **RequestLoadAsync(path, type, ...)** 加载，无按类型拆开的多套 API。
- **命名**：成员方法采用**首字母大写的驼峰**（PascalCase）；所有方法在说明列给出**完整函数签名**。
- **资源三态**：**FResource**（硬盘形态，通过 **GUID** 引用其他资源，硬盘加载使用）；**RResource**（内存形态，根据 FResource 引用通过**指针**引用其他 RResource，内存引用使用）；**DResource**（GPU 形态，**直接保存在 RResource 内部**）。部分资源可能只存在某一形态。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | — | 概念/类型 | 硬盘形态资源 | te/resource/FResource.h | FResource | 硬盘上的资源表示；引用其他资源仅通过**全局唯一 GUID**；**硬盘加载使用 FResource**；部分资源可能仅存在 F 形态 |
| 013-Resource | te::resource | — | 概念/类型 | 运行时形态资源 | te/resource/RResource.h | RResource | 内存中的资源表示；根据 FResource 的引用通过**指针**引用其他 RResource；**DResource 直接保存在 RResource 内部**；**内存引用使用 RResource**；部分资源可能仅存在 R 形态 |
| 013-Resource | te::resource | — | 概念/类型 | GPU 形态资源 | te/resource/DResource.h | DResource | **GPU 类型资源**；不单独作为跨对象引用，**保存在 RResource 内部**，由 RResource 管理生命周期与绑定 |
| 013-Resource | te::resource | — | 枚举 | 资源类型 | te/resource/ResourceTypes.h | ResourceType | `enum class ResourceType { Texture, Mesh, Material, Model, Effect, Terrain, Shader, Audio, Custom, _Count };` 所有类型均经统一 RequestLoadAsync 加载 |
| 013-Resource | te::resource | — | 枚举 | 加载状态 | te/resource/ResourceTypes.h | LoadStatus | `enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled };` 定义在 ResourceTypes.h 中 |
| 013-Resource | te::resource | — | 枚举 | 加载结果 | te/resource/ResourceTypes.h | LoadResult | `enum class LoadResult { Ok, NotFound, Error, Cancelled };` 定义在 ResourceTypes.h 中 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **统一**资源异步加载接口（线程安全） | te/resource/ResourceManager.h | IResourceManager::RequestLoadAsync | `LoadRequestId RequestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data);` 异步加载入口；创建资源实例并调用 IResource::LoadAsync；线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载状态 | te/resource/ResourceManager.h | IResourceManager::GetLoadStatus | `LoadStatus GetLoadStatus(LoadRequestId id) const;` 线程安全；Pending/Loading/Completed/Failed/Cancelled |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载进度 | te/resource/ResourceManager.h | IResourceManager::GetLoadProgress | `float GetLoadProgress(LoadRequestId id) const;` 返回 0.f～1.f；线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 取消加载 | te/resource/ResourceManager.h | IResourceManager::CancelLoad | `void CancelLoad(LoadRequestId id);` 取消未完成的请求；回调仍会触发，result 为 Cancelled |
| 013-Resource | te::resource | — | 类型别名/句柄 | 加载请求 ID | te/resource/ResourceManager.h | LoadRequestId | `using LoadRequestId = void*;` 不透明句柄，由 RequestLoadAsync 返回 |
| 013-Resource | te::resource | — | 回调类型 | 加载完成回调 | te/resource/ResourceManager.h | LoadCompleteCallback | `using LoadCompleteCallback = void (*)(IResource* resource, LoadResult result, void* user_data);` 在约定线程调用（由 IThreadPool::SetCallbackThread 指定，默认主线程） |
| 013-Resource | te::resource | IResource | 抽象基类 | 资源基类 | te/resource/Resource.h | IResource | 所有资源类型的基类；提供 Load、LoadAsync、Save、Import 方法及 protected 辅助方法；不直接构造；由 RequestLoadAsync 回调返回 |
| 013-Resource | te::resource | IResource | 抽象基类 | 查询资源类型 | te/resource/Resource.h | IResource::GetResourceType | `ResourceType GetResourceType() const;` 纯虚函数；回调拿到 IResource* 后可根据类型向下转型 |
| 013-Resource | te::resource | IResource | 抽象基类 | 获取资源 ID | te/resource/Resource.h | IResource::GetResourceId | `ResourceId GetResourceId() const;` 纯虚函数；返回资源 GUID |
| 013-Resource | te::resource | IResource | 抽象基类 | 释放资源 | te/resource/Resource.h | IResource::Release | `void Release();` 纯虚函数；递减引用计数；当为零时资源可被回收 |
| 013-Resource | te::resource | IResource | 抽象基类 | 同步加载资源 | te/resource/Resource.h | IResource::Load | `bool Load(char const* path, IResourceManager* manager);` 虚函数，有默认实现（仅验证输入并调用 OnLoadComplete）；子类应重写以调用 LoadAssetDesc<T>、LoadDataFile、LoadDependencies<T> 等 |
| 013-Resource | te::resource | IResource | 抽象基类 | 异步加载资源 | te/resource/Resource.h | IResource::LoadAsync | `bool LoadAsync(char const* path, IResourceManager* manager, LoadCompleteCallback on_done, void* user_data);` 虚函数，有默认实现（使用 IThreadPool 在后台线程执行 Load，在约定线程调用回调） |
| 013-Resource | te::resource | IResource | 抽象基类 | 保存资源 | te/resource/Resource.h | IResource::Save | `bool Save(char const* path, IResourceManager* manager);` 虚函数，有默认实现（调用 OnPrepareSave）；子类应重写以调用 SaveAssetDesc<T>、SaveDataFile 等 |
| 013-Resource | te::resource | IResource | 抽象基类 | 导入资源 | te/resource/Resource.h | IResource::Import | `bool Import(char const* sourcePath, IResourceManager* manager);` 虚函数，有默认实现（调用 DetectFormat、OnConvertSourceFile、OnCreateAssetDesc、GenerateGUID、SaveAssetDesc、SaveDataFile） |
| 013-Resource | te::resource | IResource | 抽象基类 | 创建设备资源 | te/resource/Resource.h | IResource::EnsureDeviceResources | `void EnsureDeviceResources();` 虚函数，默认实现为空；创建 GPU 资源（DResource）；013 不参与，由子类调用 008-RHI |
| 013-Resource | te::resource | IResource | 抽象基类 | 异步创建设备资源 | te/resource/Resource.h | IResource::EnsureDeviceResourcesAsync | `void EnsureDeviceResourcesAsync(void (*on_done)(void*), void* user_data);` 虚函数，默认实现为空；异步创建 GPU 资源 |
| 013-Resource | te::resource | IResource | 保护模板方法 | 加载 AssetDesc | te/resource/Resource.h | IResource::LoadAssetDesc<T> | `template<typename T> std::unique_ptr<T> LoadAssetDesc(char const* path);` protected 模板方法；读取 AssetDesc 文件并反序列化（调用 002-Object）；需要 AssetDescTypeName<T> 特化 |
| 013-Resource | te::resource | IResource | 保护模板方法 | 保存 AssetDesc | te/resource/Resource.h | IResource::SaveAssetDesc<T> | `template<typename T> bool SaveAssetDesc(char const* path, T const* desc);` protected 模板方法；序列化 AssetDesc 并写入文件（调用 002-Object）；需要 AssetDescTypeName<T> 特化 |
| 013-Resource | te::resource | IResource | 保护模板方法 | 加载依赖 | te/resource/Resource.h | IResource::LoadDependencies<T, GetDepsFn> | `template<typename T, typename GetDepsFn> bool LoadDependencies(T const* desc, GetDepsFn getDeps, IResourceManager* manager);` protected 模板方法；从 AssetDesc 提取依赖列表并加载；自动选择同步/异步模式 |
| 013-Resource | te::resource | IResource | 保护方法 | 加载单个依赖 | te/resource/Resource.h | IResource::LoadDependency | `IResource* LoadDependency(ResourceId guid, IResourceManager* manager);` protected 方法；加载单个依赖资源（同步，递归） |
| 013-Resource | te::resource | IResource | 保护方法 | 读取数据文件 | te/resource/Resource.h | IResource::LoadDataFile | `bool LoadDataFile(char const* path, void** outData, std::size_t* outSize);` protected 方法；读取数据文件（调用 001-Core FileRead）；调用方必须释放 outData |
| 013-Resource | te::resource | IResource | 保护方法 | 写入数据文件 | te/resource/Resource.h | IResource::SaveDataFile | `bool SaveDataFile(char const* path, void const* data, std::size_t size);` protected 方法；写入数据文件（调用 001-Core FileWrite） |
| 013-Resource | te::resource | IResource | 保护方法 | 生成 GUID | te/resource/Resource.h | IResource::GenerateGUID | `ResourceId GenerateGUID();` protected 方法；生成 GUID（调用 002-Object GUID::Generate） |
| 013-Resource | te::resource | IResource | 保护方法 | 检测格式 | te/resource/Resource.h | IResource::DetectFormat | `std::string DetectFormat(char const* sourcePath);` protected 方法；检测源文件格式（通过文件扩展名，调用 001-Core PathGetExtension） |
| 013-Resource | te::resource | IResource | 保护方法 | 获取 AssetDesc 路径 | te/resource/Resource.h | IResource::GetDescPath | `std::string GetDescPath(char const* path) const;` protected 方法；从资源路径生成 AssetDesc 文件路径 |
| 013-Resource | te::resource | IResource | 保护方法 | 获取数据文件路径 | te/resource/Resource.h | IResource::GetDataPath | `std::string GetDataPath(char const* path) const;` protected 方法；从资源路径生成数据文件路径（添加 .data 扩展） |
| 013-Resource | te::resource | IResource | 保护虚函数 | 加载完成钩子 | te/resource/Resource.h | IResource::OnLoadComplete | `virtual void OnLoadComplete();` protected 虚函数，默认实现为空；子类可重写以执行资源特定初始化 |
| 013-Resource | te::resource | IResource | 保护虚函数 | 保存准备钩子 | te/resource/Resource.h | IResource::OnPrepareSave | `virtual void OnPrepareSave();` protected 虚函数，默认实现为空；子类可重写以准备保存数据 |
| 013-Resource | te::resource | IResource | 保护纯虚函数 | 转换源文件 | te/resource/Resource.h | IResource::OnConvertSourceFile | `virtual bool OnConvertSourceFile(char const* sourcePath, void** outData, std::size_t* outSize) = 0;` protected 纯虚函数；子类必须实现；转换源文件为引擎格式 |
| 013-Resource | te::resource | IResource | 保护纯虚函数 | 创建 AssetDesc | te/resource/Resource.h | IResource::OnCreateAssetDesc | `virtual void* OnCreateAssetDesc() = 0;` protected 纯虚函数；子类必须实现；创建 AssetDesc 实例（通过 002-Object） |
| 013-Resource | te::resource | — | 类型特征 | AssetDesc 类型名 | te/resource/Resource.inl | AssetDescTypeName<T> | `template<typename T> struct AssetDescTypeName { static const char* Get(); };` 类型特征；资源模块应为各自的 AssetDesc 类型特化此特征 |
| 013-Resource | te::resource | ITextureResource | 抽象接口 | 纹理资源视图 | te/resource/TextureResource.h | ITextureResource | 纹理宽高、格式、GPU 句柄等；由 requestLoadAsync(..., ResourceType::Texture, ...) 回调返回或 IResource 转型；由 028-Texture 实现 |
| 013-Resource | te::resource | IMeshResource | 抽象接口 | 网格资源视图 | te/resource/MeshResource.h | IMeshResource | 顶点/索引、子网格、LOD 等；Mesh 来源于 OBJ、FBX 等常用格式；按类型 Mesh 加载或经 Model 资源引用；由 012-Mesh 实现 |
| 013-Resource | te::resource | IMaterialResource | 抽象接口 | 材质资源视图 | te/resource/MaterialResource.h | IMaterialResource | **引擎自有格式**；材质**保存 Shader**，并引用**贴图**、**材质参数**（渲染 Shader 的参数值）；按类型 Material 加载或经 Model 资源引用；由 011-Material 实现 |
| 029-World | te::world | IModelResource | 抽象接口 | 模型资源视图 | te/world/ModelResource.h | IModelResource | **归属 029-World**。硬盘上的 Model 资源引用了 Material 和 Mesh；聚合若干 Mesh 与若干 Material 的引用；经 013 统一 requestLoadAsync(..., Model, ...) 加载后返回；模型渲染 = Mesh + Material 的组织 |
| 013-Resource | te::resource | IEffectResource | 抽象接口 | 特效资源视图 | te/resource/EffectResource.h | IEffectResource | 粒子/VFX 等；经**统一接口** requestLoadAsync(..., Effect, ...) 加载 |
| 013-Resource | te::resource | ITerrainResource | 抽象接口 | 地形资源视图 | te/resource/TerrainResource.h | ITerrainResource | 地形块/高度图等；经**统一接口** requestLoadAsync(..., Terrain, ...) 加载 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 注册资源工厂 | te/resource/ResourceManager.h | IResourceManager::RegisterResourceFactory | `void RegisterResourceFactory(ResourceType type, ResourceFactory factory);` 注册资源工厂函数，用于创建 IResource 实例；ResourceFactory = IResource* (*)(ResourceType)；实现采用混合机制：优先使用 002-Object TypeRegistry::CreateInstance，回退到 ResourceFactory |
| 013-Resource | te::resource | — | 类型别名 | 资源工厂函数 | te/resource/ResourceManager.h | ResourceFactory | `using ResourceFactory = IResource* (*)(ResourceType);` 资源工厂函数指针类型 |
| 013-Resource | te::resource | — | 自由函数 | 获取全局 ResourceManager | te/resource/ResourceManager.h | GetResourceManager | `IResourceManager* GetResourceManager();` 由 Subsystems 注册或单例；调用方不拥有指针 |
| 013-Resource | te::resource | — | 类型别名 | 资源全局唯一 ID | te/resource/ResourceId.h | ResourceId | `using ResourceId = object::GUID;` 等价 GUID；FResource 间引用、可寻址路径、与 Object 引用解析对接 |
| 013-Resource | te::resource | — | 特化 | ResourceId 哈希 | te/resource/ResourceId.h | std::hash<ResourceId> | `template<> struct std::hash<te::resource::ResourceId>;` std::hash 特化，用于 unordered_map 等容器 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 同步加载 | te/resource/ResourceManager.h | IResourceManager::LoadSync | `IResource* LoadSync(char const* path, ResourceType type);` 同步加载入口；创建资源实例并调用 IResource::Load；阻塞直至完成；失败返回 nullptr；线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 释放/卸载 | te/resource/ResourceManager.h | IResourceManager::Unload, IResource::Release | `void Unload(IResource* resource);` `void IResource::Release();` 与各模块句柄协调；卸载策略由实现约定；Unload 递减引用计数，当为零时从缓存移除 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 流式请求与优先级 | te/resource/ResourceManager.h | IResourceManager::RequestStreaming, SetStreamingPriority | `StreamingHandle RequestStreaming(ResourceId id, int priority);` `void SetStreamingPriority(StreamingHandle h, int priority);` 与 LOD/Terrain 对接；当前为占位实现 |
| 013-Resource | te::resource | — | 类型别名/句柄 | 流式句柄 | te/resource/ResourceManager.h | StreamingHandle | `using StreamingHandle = void*;` 不透明句柄，用于流式加载 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 缓存查询 | te/resource/ResourceManager.h | IResourceManager::GetCached | `IResource* GetCached(ResourceId id) const;` 仅查缓存，未命中返回 nullptr，不触发加载；线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | Import | te/resource/ResourceManager.h | IResourceManager::Import | `bool Import(char const* path, ResourceType type, void* out_metadata_or_null);` 创建资源实例并调用 IResource::Import |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | Save | te/resource/ResourceManager.h | IResourceManager::Save | `bool Save(IResource* resource, char const* path);` 调用 IResource::Save |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 寻址解析 | te/resource/ResourceManager.h | IResourceManager::ResolvePath | `char const* ResolvePath(ResourceId id) const;` GUID→路径；未解析返回 nullptr；线程安全 |

*来源：用户故事 US-resource-001/002/003。契约能力：Import、Load、Unload、Streaming、Addressing（ResourceId/GUID）。*
