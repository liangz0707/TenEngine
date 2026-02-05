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
| 013-Resource | te::resource | — | 枚举 | 资源类型 | te/resource/ResourceTypes.h | ResourceType | `enum class ResourceType { Texture, Mesh, Material, Model, Effect, Terrain, Shader, Audio, Custom, … };` 所有类型均经统一 RequestLoadAsync 加载 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **统一**资源加载接口（线程安全） | te/resource/ResourceManager.h | IResourceManager::RequestLoadAsync | `LoadRequestId RequestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data);` 唯一加载入口；线程安全。可选重载 `RequestLoadAsync(path, on_done, user_data)` 由路径/扩展名推断 type |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载状态 | te/resource/ResourceManager.h | IResourceManager::GetLoadStatus | `LoadStatus GetLoadStatus(LoadRequestId id) const;` 线程安全；Pending/Loading/Completed/Failed/Cancelled |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载进度 | te/resource/ResourceManager.h | IResourceManager::GetLoadProgress | `float GetLoadProgress(LoadRequestId id) const;` 返回 0.f～1.f；线程安全 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 取消加载 | te/resource/ResourceManager.h | IResourceManager::CancelLoad | `void CancelLoad(LoadRequestId id);` 取消未完成的请求；回调仍会触发，result 为 Cancelled |
| 013-Resource | te::resource | — | 类型别名/句柄 | 加载请求 ID | te/resource/ResourceManager.h | LoadRequestId | 不透明句柄，由 RequestLoadAsync 返回 |
| 013-Resource | te::resource | — | 枚举 | 加载状态 | te/resource/ResourceManager.h | LoadStatus | `enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled };` |
| 013-Resource | te::resource | — | 回调类型 | 加载完成回调 | te/resource/ResourceManager.h | LoadCompleteCallback | `void (*LoadCompleteCallback)(IResource* resource, LoadResult result, void* user_data);` 在约定线程调用 |
| 013-Resource | te::resource | IResource | 抽象接口 | 资源句柄 | te/resource/Resource.h | IResource | 不直接构造；由 RequestLoadAsync 回调返回 |
| 013-Resource | te::resource | IResource | 抽象接口 | 查询资源类型 | te/resource/Resource.h | IResource::GetResourceType | `ResourceType GetResourceType() const;` 回调拿到 IResource* 后可根据类型向下转型 |
| 013-Resource | te::resource | ITextureResource | 抽象接口 | 纹理资源视图 | te/resource/TextureResource.h | ITextureResource | 纹理宽高、格式、GPU 句柄等；由 requestLoadAsync(..., ResourceType::Texture, ...) 回调返回或 IResource 转型 |
| 013-Resource | te::resource | IMeshResource | 抽象接口 | 网格资源视图 | te/resource/MeshResource.h | IMeshResource | 顶点/索引、子网格、LOD 等；Mesh 来源于 OBJ、FBX 等常用格式；按类型 Mesh 加载或经 Model 资源引用 |
| 013-Resource | te::resource | IMaterialResource | 抽象接口 | 材质资源视图 | te/resource/MaterialResource.h | IMaterialResource | **引擎自有格式**；材质**保存 Shader**，并引用**贴图**、**材质参数**（渲染 Shader 的参数值）；按类型 Material 加载或经 Model 资源引用 |
| 013-Resource | te::resource | IModelResource | 抽象接口 | 模型资源视图 | te/resource/ModelResource.h | IModelResource | **硬盘上的 Model 资源引用了 Material 和 Mesh**；聚合若干 Mesh 与若干 Material 的引用；经**统一接口** requestLoadAsync(..., Model, ...) 加载；模型渲染 = Mesh + Material 的组织 |
| 013-Resource | te::resource | IEffectResource | 抽象接口 | 特效资源视图 | te/resource/EffectResource.h | IEffectResource | 粒子/VFX 等；经**统一接口** requestLoadAsync(..., Effect, ...) 加载 |
| 013-Resource | te::resource | ITerrainResource | 抽象接口 | 地形资源视图 | te/resource/TerrainResource.h | ITerrainResource | 地形块/高度图等；经**统一接口** requestLoadAsync(..., Terrain, ...) 加载 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 可选：注册自定义类型加载器 | te/resource/ResourceManager.h | IResourceManager::RegisterResourceLoader | `void RegisterResourceLoader(ResourceType type, IResourceLoader* loader);` 扩展新类型后仍用统一 RequestLoadAsync 加载 |
| 013-Resource | te::resource | — | 枚举 | 加载结果 | te/resource/ResourceManager.h | LoadResult | `enum class LoadResult { Ok, NotFound, Error, Cancelled };` |
| 013-Resource | te::resource | — | 自由函数 | 获取全局 ResourceManager | te/resource/ResourceManager.h | GetResourceManager | `IResourceManager* GetResourceManager();` 由 Subsystems 注册或单例；调用方不拥有指针 |
| 013-Resource | te::resource | — | 类型别名 | 资源全局唯一 ID | te/resource/ResourceId.h | ResourceId | 等价 GUID；FResource 间引用、可寻址路径、与 Object 引用解析对接 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 同步加载（可选） | te/resource/ResourceManager.h | IResourceManager::LoadSync | `IResource* LoadSync(char const* path, ResourceType type);` 阻塞直至完成；失败返回 nullptr |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 释放/卸载 | te/resource/ResourceManager.h | IResourceManager::Unload, IResource::Release | `void Unload(IResource* resource);` `void IResource::Release();` 与各模块句柄协调；卸载策略由实现约定 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 流式请求与优先级 | te/resource/ResourceManager.h | IResourceManager::RequestStreaming, SetStreamingPriority | `StreamingHandle RequestStreaming(ResourceId id, int priority);` `void SetStreamingPriority(StreamingHandle h, int priority);` 与 LOD/Terrain 对接 |

*来源：用户故事 US-resource-001/002/003。契约能力：Import、Load、Unload、Streaming、Addressing（ResourceId/GUID）。*

---

## 数据相关 TODO

（依据 [docs/assets/resource-serialization.md](../../docs/assets/resource-serialization.md)、[013-resource-data-model.md](../../docs/assets/013-resource-data-model.md)；本模块上游：001-Core、002-Object。）

### 注册

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] `RegisterResourceLoader(type, IResourceLoader*)` | — |
| [ ] 引擎启动时注册各描述类型到 002（TextureAssetDesc、ModelAssetDesc、MaterialAssetDesc、MeshAssetDesc、LevelAssetDesc 等） | 002：`RegisterType<T>` |
| [ ] 各类型 Loader（Texture/Mesh/Material/Model）向 013 注册 | — |

### 序列化

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] `Save(resource, path)`：写出引擎格式到磁盘 | 002：`Serialize(obj, buf, typeId)`（.texture、.material、.model、.level） |
| [ ] 若方案 B：Import/Save 时更新 Manifest（GUID→path） | 001：`FileWrite` |

### 反序列化

| 需提供 | 需调用上游 |
|--------|------------|
| [ ] `RequestLoadAsync(ResourceId, type)` / `LoadSync(ResourceId, type)` | 002：`GetTypeByName` → `Deserialize(buf, obj, typeId)` |
| [ ] GUID→路径解析：实现 §3.1 方案 A（约定派生）或 B（Manifest） | 001：`FileRead`（读描述文件、.texdata、.mesh） |

### 数据

- [ ] **FResource**：磁盘形态；引用仅 GUID
- [ ] **RResource**：内存形态；内持描述、DResource 槽位（可空）
- [ ] **DResource**：GPU 形态；保存在 RResource 内部
- [ ] Load 阶段仅建 RResource；DResource 在 EnsureDeviceResourcesAsync 时按需创建

### 需提供的对外接口

| 接口 | 说明 |
|------|------|
| [ ] `RequestLoadAsync(ResourceId, type, callback)` / `LoadSync(ResourceId, type)` | 入参 GUID + ResourceType；内部 GUID→path 后按类型加载 |
| [ ] `EnsureDeviceResourcesAsync(IResource*)` | 对该资源及引用异步创建 DResource；委托 IResourceLoader |
| [ ] `IsDeviceReady(IResource*)` / `HasDResource(IResource*)` | 查询 DResource 是否可用 |
| [ ] `Unload(IResource*)` / `IResource::Release()` | 释放引用 |
| [ ] `Import(path, type)` / `Save(resource, path)` | 外部格式→引擎格式；写出 .texture/.texdata/.mesh/.material/.model/.level |

### 需调用上游

| 场景 | 调用 002 接口 | 调用 001 接口 |
|------|---------------|---------------|
| Load 描述文件 | `GetTypeByName` → `Deserialize` | `FileRead` |
| Save 描述文件 | `Serialize` | `FileWrite` |
| 读 .texdata/.mesh | — | `FileRead` |

### 调用流程

1. **Load**：RequestLoadAsync(ResourceId, type) → GUID→path → 001.FileRead → 002.Deserialize → 按类型创建 RResource（委托 Loader 调 011/012）→ 回调返回 IResource*
2. **无引擎格式回退**：若目录内无 .texture/.mesh → Import→Save → 再 Load
3. **EnsureDeviceResources**：020 收集到 Model → 调用 013.EnsureDeviceResourcesAsync(IResource*) → 013 委托 Loader 对 Mesh/Material/Texture 调 012.EnsureDeviceResources、011.EnsureDeviceResources
4. **绘制前**：020 调用 013.IsDeviceReady(resource)；false 则跳过/等待/占位
