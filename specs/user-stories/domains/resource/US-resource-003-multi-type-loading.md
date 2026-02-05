# US-resource-003：所有资源使用统一资源加载接口（Mesh、Texture、Material、Model、Effect、Terrain 等）

- **标题**：**所有资源**均通过**统一资源加载接口**加载，包括 Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等；同一 API（requestLoadAsync）按 ResourceType 区分类型，回调中返回统一 IResource（可查类型后向下转型或访问类型化接口）。
- **编号**：US-resource-003

---

## 1. 角色/触发

- **角色**：游戏逻辑、渲染管线、编辑器
- **触发**：需要加载任意类型资源（Mesh、Texture、Material、Model、Effect、Terrain 等）；**统一**使用同一套加载接口（**requestLoadAsync(path, type, callback, user_data)**），不按类型拆成多套 API；加载完成后在回调中拿到统一 **IResource***，再通过 **getResourceType()** 与类型化接口使用。

---

## 2. 端到端流程

1. **统一接口**：所有资源类型（Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等）均通过 **同一** **requestLoadAsync(path, type, callback, user_data)** 加载；**不**提供按类型拆开的 requestLoadMeshAsync、requestLoadTextureAsync 等多套 API，仅通过 **ResourceType** 参数区分。
2. **Resource** 模块根据 **ResourceType** 选择对应加载器（TextureLoader、MeshLoader、MaterialLoader、ModelLoader、EffectLoader、TerrainLoader 等），在专用加载线程执行；语义与 US-resource-001/002 一致（线程安全、回调、状态/进度）。
3. 加载完成后，回调中统一返回 **IResource***；调用方通过 **IResource::getResourceType()** 得到类型，再向下转型或通过 **ITextureResource**、**IMeshResource**、**IMaterialResource**、**IModelResource**、**IEffectResource**、**ITerrainResource** 等类型化接口访问。
4. 下游（渲染、实体、编辑器）按类型使用：纹理交给材质或贴图槽，网格交给 ModelComponent，Effect 交给 EffectComponent，Terrain 交给 TerrainComponent，依实现约定。
5. **资源类型可扩展**：引擎内置 Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等；新类型可通过 **registerResourceLoader(type, loader)** 扩展，仍使用同一 **requestLoadAsync** 入口。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 013-Resource | **统一资源加载接口** requestLoadAsync(path, type, ...)；ResourceType 枚举（Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等）；IResource::getResourceType；类型化资源接口（ITextureResource、IMeshResource、IMaterialResource、IModelResource、IEffectResource、ITerrainResource 等）；可扩展加载器注册 |
| 002-Object | 可选：新资源类型注册与反射，便于编辑器与序列化 |

---

## 4. 每模块职责与 I/O

### 013-Resource

- **职责**：提供**统一资源加载接口** **requestLoadAsync(path, type, callback, user_data)**，**所有**资源类型（Mesh、Texture、Material、Model、Effect、Terrain、Shader、Audio 等）均通过此唯一入口加载；定义 **ResourceType** 枚举（含 Effect、Terrain 等，可扩展）；按 type 分发到对应加载器；**IResource::getResourceType()** 返回类型；提供类型化资源接口 **ITextureResource**、**IMeshResource**、**IMaterialResource**、**IModelResource**、**IEffectResource**、**ITerrainResource** 等，供渲染与实体模块使用；可选 **registerResourceLoader(type, loader)** 扩展新类型，扩展后仍使用同一 requestLoadAsync。
- **输入**：path、ResourceType、LoadCompleteCallback、user_data；扩展时 registerResourceLoader(type, loader)。
- **输出**：ResourceType 枚举；**统一** requestLoadAsync(path, type, ...)；IResource::getResourceType；ITextureResource、IMeshResource、IMaterialResource、IModelResource、IEffectResource、ITerrainResource 等；可选 registerResourceLoader。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 013-Resource

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | TenEngine::resource | — | 枚举 | 资源类型 | TenEngine/resource/ResourceTypes.h | ResourceType | enum class ResourceType { Texture, Mesh, Material, Model, Effect, Terrain, Shader, Audio, Custom, … }; **所有**资源类型均通过统一接口加载 |
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | **统一**资源加载接口（线程安全） | TenEngine/resource/ResourceManager.h | IResourceManager::requestLoadAsync | LoadRequestId requestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data); **唯一**加载入口，Mesh/Texture/Material/Model/Effect/Terrain 等均用此 API；按 type 选择加载器 |
| 013-Resource | TenEngine::resource | IResource | 抽象接口 | 查询资源类型 | TenEngine/resource/Resource.h | IResource::getResourceType | ResourceType getResourceType() const; 回调拿到 IResource* 后可根据类型向下转型或访问类型化接口 |
| 013-Resource | TenEngine::resource | ITextureResource | 抽象接口 | 纹理资源视图 | TenEngine/resource/TextureResource.h | ITextureResource | 继承或可从 IResource 按 getResourceType()==Texture 得到；纹理宽高、格式、GPU 句柄等由实现提供 |
| 013-Resource | TenEngine::resource | IMeshResource | 抽象接口 | 网格资源视图 | TenEngine/resource/MeshResource.h | IMeshResource | 顶点/索引、子网格、LOD 等；由 requestLoadAsync(..., ResourceType::Mesh, ...) 回调返回或 IResource 转型 |
| 013-Resource | TenEngine::resource | IMaterialResource | 抽象接口 | 材质资源视图 | TenEngine/resource/MaterialResource.h | IMaterialResource | 材质参数、纹理槽、Shader 引用等；按类型加载 Material 时使用 |
| 013-Resource | TenEngine::resource | IModelResource | 抽象接口 | 模型资源视图 | TenEngine/resource/ModelResource.h | IModelResource | 聚合 Mesh/Material 等子资源；经**统一接口** requestLoadAsync(..., Model, ...) 加载 |
| 013-Resource | TenEngine::resource | IEffectResource | 抽象接口 | 特效资源视图 | TenEngine/resource/EffectResource.h | IEffectResource | 粒子/VFX 等；经**统一接口** requestLoadAsync(..., Effect, ...) 加载 |
| 013-Resource | TenEngine::resource | ITerrainResource | 抽象接口 | 地形资源视图 | TenEngine/resource/TerrainResource.h | ITerrainResource | 地形块/高度图等；经**统一接口** requestLoadAsync(..., Terrain, ...) 加载 |
| 013-Resource | TenEngine::resource | IResourceManager | 抽象接口 | 可选：注册自定义类型加载器 | TenEngine/resource/ResourceManager.h | IResourceManager::registerResourceLoader | void registerResourceLoader(ResourceType type, IResourceLoader* loader); 扩展新类型后仍用**统一** requestLoadAsync 加载 |

（**Shader**、**Audio** 等类型可同样增加 IShaderResource、IAudioResource；均经**统一** requestLoadAsync(path, type, ...) 加载。LoadRequestId、getLoadStatus、getLoadProgress、cancelLoad、LoadCompleteCallback、LoadResult 同 US-resource-001/002。）

---

## 6. 参考（可选）

- **Unity**：AssetBundle / Addressables 按类型加载（Texture2D、Mesh、Material、GameObject 等）；AssetType 与 Get/Load 泛型。
- **Unreal**：UStaticMesh、UMaterial、UTexture 等 UObject 类型；StreamableManager 按路径与类型加载。
- **通用**：按 MIME 或扩展名选择加载器；类型化句柄与统一 IResource + getType 并存。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/013-resource-ABI.md`。与 US-resource-001/002 互补：001 为异步加载与回调，002 为多线程与状态/进度，003 为**统一资源加载接口**——所有资源类型（Mesh、Texture、Material、Model、Effect、Terrain 等）均通过同一 requestLoadAsync 加载及类型化接口。*
