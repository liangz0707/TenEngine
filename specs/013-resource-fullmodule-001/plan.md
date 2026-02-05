# Implementation Plan: 013-Resource 完整模块实现

**Branch**: `013-resource-fullmodule-001` | **Date**: 2026-02-05 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/013-resource-fullmodule-001/spec.md`

## Summary

实现 013-Resource 模块的**完整能力**：IResource 基类与 ResourceType、统一加载（RequestLoadAsync/LoadSync）、资源缓存（GetCached，引用计数）、加载工具（GetLoadStatus/GetLoadProgress/CancelLoad、RequestStreaming/SetStreamingPriority）、寻址（ResourceId、ResolvePath）、卸载（Unload/Release）、EnsureDeviceResources 转发、导入/序列化/Save/Load 统一接口（各模块实现 Importer/Loader/反序列化器，013 调度与磁盘 I/O）。技术方案：C++17，依赖 001-Core、002-Object、028-Texture 源码构建；反序列化由 013 按 ResourceType 调各模块注册的反序列化器；Load 采用不透明 payload 传递；循环引用禁止；GetCached 仅查缓存；异步回调仅在「已加载」时调用一次。见 [research.md](./research.md)。

## 实现范围（TenEngine：实现全量 ABI 内容）

本 feature 须实现 **`specs/_contracts/013-resource-ABI.md`** 中已有符号，以及 public-api TODO 所涉的**新增**接口（GetCached、Save、Import、RegisterImporter、ResolvePath、IResourceLoader、IResourceImporter、反序列化器注册）。下方「全量 ABI 内容」表为**原始 ABI + 本 feature 新增**，tasks/implement 以此为准。

### 全量 ABI 内容（实现参考）

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 013-Resource | te::resource | — | 概念/类型 | 硬盘形态资源 | te/resource/FResource.h | FResource | 硬盘上的资源表示；引用其他资源仅通过全局唯一 GUID |
| 013-Resource | te::resource | — | 概念/类型 | 运行时形态资源 | te/resource/RResource.h | RResource | 内存中的资源表示；DResource 直接保存在 RResource 内部 |
| 013-Resource | te::resource | — | 概念/类型 | GPU 形态资源 | te/resource/DResource.h | DResource | GPU 类型资源；保存在 RResource 内部，由 RResource 管理 |
| 013-Resource | te::resource | — | 枚举 | 资源类型 | te/resource/ResourceTypes.h | ResourceType | `enum class ResourceType { Texture, Mesh, Material, Model, Effect, Terrain, Shader, Audio, Custom, … };` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 统一资源加载（线程安全） | te/resource/ResourceManager.h | IResourceManager::RequestLoadAsync | `LoadRequestId RequestLoadAsync(char const* path, ResourceType type, LoadCompleteCallback on_done, void* user_data);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载状态 | te/resource/ResourceManager.h | IResourceManager::GetLoadStatus | `LoadStatus GetLoadStatus(LoadRequestId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 查询加载进度 | te/resource/ResourceManager.h | IResourceManager::GetLoadProgress | `float GetLoadProgress(LoadRequestId id) const;` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 取消加载 | te/resource/ResourceManager.h | IResourceManager::CancelLoad | `void CancelLoad(LoadRequestId id);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **缓存查询** | te/resource/ResourceManager.h | IResourceManager::GetCached | `IResource* GetCached(ResourceId id) const;` 仅查缓存，未命中返回 nullptr，不触发加载 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 同步加载 | te/resource/ResourceManager.h | IResourceManager::LoadSync | `IResource* LoadSync(char const* path, ResourceType type);` 失败返回 nullptr |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 释放/卸载 | te/resource/ResourceManager.h | IResourceManager::Unload, IResource::Release | `void Unload(IResource* resource);` `void IResource::Release();` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 流式请求与优先级 | te/resource/ResourceManager.h | IResourceManager::RequestStreaming, SetStreamingPriority | `StreamingHandle RequestStreaming(ResourceId id, int priority);` `void SetStreamingPriority(StreamingHandle h, int priority);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | 注册 Loader | te/resource/ResourceManager.h | IResourceManager::RegisterResourceLoader | `void RegisterResourceLoader(ResourceType type, IResourceLoader* loader);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **注册反序列化器** | te/resource/ResourceManager.h | IResourceManager::RegisterDeserializer | `void RegisterDeserializer(ResourceType type, IDeserializer* deserializer);` 各模块注册，013 按 type 调用得到 opaque payload |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **注册 Importer** | te/resource/ResourceManager.h | IResourceManager::RegisterImporter | `void RegisterImporter(ResourceType type, IResourceImporter* importer);` |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **Import** | te/resource/ResourceManager.h | IResourceManager::Import | `bool Import(char const* path, ResourceType type, void* out_metadata_or_null);` 按 type 分发到已注册 Importer |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **Save** | te/resource/ResourceManager.h | IResourceManager::Save | `bool Save(IResource* resource, char const* path);` 各模块产出内存内容，013 统一写盘 |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | **寻址解析** | te/resource/ResourceManager.h | IResourceManager::ResolvePath | `char const* ResolvePath(ResourceId id) const;` 或等价；GUID→路径；未解析返回 nullptr |
| 013-Resource | te::resource | — | 类型别名/句柄 | 加载请求 ID | te/resource/ResourceManager.h | LoadRequestId | 不透明句柄 |
| 013-Resource | te::resource | — | 枚举 | 加载状态 | te/resource/ResourceManager.h | LoadStatus | `enum class LoadStatus { Pending, Loading, Completed, Failed, Cancelled };` |
| 013-Resource | te::resource | — | 回调类型 | 加载完成回调 | te/resource/ResourceManager.h | LoadCompleteCallback | `void (*LoadCompleteCallback)(IResource* resource, LoadResult result, void* user_data);` 根及递归依赖均完成后调用一次 |
| 013-Resource | te::resource | IResource | 抽象接口 | 资源句柄 | te/resource/Resource.h | IResource | GetResourceType()、Release() |
| 013-Resource | te::resource | IResource | 抽象接口 | 查询资源类型 | te/resource/Resource.h | IResource::GetResourceType | `ResourceType GetResourceType() const;` |
| 013-Resource | te::resource | IResourceLoader | 抽象接口 | **Loader（各模块实现）** | te/resource/ResourceLoader.h | IResourceLoader::CreateFromPayload | `IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager);` 接收不透明 payload，创建 IResource 并返回；依赖通过 manager 加载 |
| 013-Resource | te::resource | IResourceImporter | 抽象接口 | **Importer（各模块实现）** | te/resource/ResourceImporter.h | IResourceImporter | DetectFormat、Convert、产出描述/数据、Metadata、Dependencies |
| 013-Resource | te::resource | IDeserializer | 抽象接口 | **反序列化器（各模块实现）** | te/resource/Deserializer.h | IDeserializer::Deserialize | `void* Deserialize(void const* buffer, size_t size);` 产出 opaque payload，013 不解析 |
| 013-Resource | te::resource | ITextureResource | 抽象接口 | 纹理资源视图 | te/resource/TextureResource.h | ITextureResource | 由 028 等实现，013 返回 IResource* 后转型 |
| 013-Resource | te::resource | IMeshResource | 抽象接口 | 网格资源视图 | te/resource/MeshResource.h | IMeshResource | 由 012 等实现 |
| 013-Resource | te::resource | IMaterialResource | 抽象接口 | 材质资源视图 | te/resource/MaterialResource.h | IMaterialResource | 由 011 等实现 |
| 029-World | te::world | IModelResource | 抽象接口 | 模型资源视图 | te/world/ModelResource.h | IModelResource | 归属 029-World，013 返回 IResource* 后转型 |
| 013-Resource | te::resource | IEffectResource | 抽象接口 | 特效资源视图 | te/resource/EffectResource.h | IEffectResource | 粒子/VFX 等 |
| 013-Resource | te::resource | ITerrainResource | 抽象接口 | 地形资源视图 | te/resource/TerrainResource.h | ITerrainResource | 地形块/高度图等 |
| 013-Resource | te::resource | — | 枚举 | 加载结果 | te/resource/ResourceManager.h | LoadResult | `enum class LoadResult { Ok, NotFound, Error, Cancelled };` |
| 013-Resource | te::resource | — | 自由函数 | 获取全局 ResourceManager | te/resource/ResourceManager.h | GetResourceManager | `IResourceManager* GetResourceManager();` |
| 013-Resource | te::resource | — | 类型别名 | 资源全局唯一 ID | te/resource/ResourceId.h | ResourceId | 等价 GUID |
| 013-Resource | te::resource | IResourceManager | 抽象接口 | EnsureDeviceResources 转发 | te/resource/Resource.h 或 ResourceManager.h | IResource::EnsureDeviceResources / EnsureDeviceResourcesAsync | 由下游触发，013 转发给 IResource 实现，013 不调用 008 |

*上表为全量 ABI（原始 + 本 feature 新增）；实现须覆盖全部符号。*

## Technical Context

**Language/Version**: C++17（与 Constitution 一致）  
**Primary Dependencies**: 001-Core（文件、内存、异步）、002-Object（序列化、反射、GUID）、028-Texture（CreateTexture 等）；仅使用各契约已声明的类型与 API。  
**Storage**: 无模块自有持久化存储；缓存为内存（ResourceId → IResource*）；文件 I/O 通过 001-Core 或平台 API。  
**Testing**: 单元测试（缓存、引用计数、LoadSync/Load 流程、循环检测）；集成测试（与上游 001/002/028 对接、Loader/Importer 注册与调用）；契约测试（ABI 符号与行为）。  
**Target Platform**: Windows、Linux、macOS（与引擎一致）。  
**Project Type**: 引擎子模块（单库）；include/te/resource/、src/、tests/。  
**Constraints**: 013 不创建、不调用 008-RHI；依赖图无环；GetCached 不触发加载；回调仅在「已加载」时一次。

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | 通过 TenEngineHelpers / tenengine_resolve_my_dependencies 引入上游源码构建（同级 worktree 或 TENENGINE_001_CORE_DIR）。 |
| 002-object | **源码** | 同上，引入 002 源码构建。 |
| 028-texture | **源码** | 同上，引入 028 源码构建。 |

**说明**：当前所有子模块构建均使用源码方式；构建根目录为当前 worktree 根（如 `TenEngine-013-resource`）；不在未澄清前执行 cmake 生成。

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | 013-resource-public-api.md 未声明第三方库。 |

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 |
|------|------|
| §VI 契约与 ABI 对齐 | 通过：实现仅暴露 `specs/_contracts/013-resource-public-api.md` 与 ABI 中声明的类型与接口；全量 ABI 已列于本 plan。 |
| §VI 全量 ABI 实现 | 通过：本 feature 实现上表全部符号，无长期 stub。 |
| §VI 构建引入真实子模块 | 通过：依赖 001、002、028 以源码引入，禁止以 stub 代替。 |
| §VI 无 stub 长期方案 | 通过：不采用仅返回 null 的占位实现作为正式实现。 |
| 契约更新流程 | 通过：新增/修改条目已写入下方「契约更新」小节，写回时仅增补/替换到 013-resource-ABI.md。 |
| 语言与构建 | 通过：C++17，CMake，与 constitution 一致。 |

*Phase 1 设计后无新增违反项。*

## Project Structure

### Documentation (this feature)

```text
specs/013-resource-fullmodule-001/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/          # ABI 全量在 plan 内，contracts 仅说明
└── tasks.md             # /speckit.tasks 产出
```

### Source Code (repository root)

```text
TenEngine-013-resource/   # worktree 根 = 构建根
├── CMakeLists.txt
├── include/
│   └── te/
│       └── resource/
│           ├── ResourceTypes.h
│           ├── ResourceId.h
│           ├── Resource.h
│           ├── ResourceManager.h
│           ├── ResourceLoader.h
│           ├── ResourceImporter.h
│           ├── Deserializer.h
│           ├── FResource.h
│           ├── RResource.h
│           ├── DResource.h
│           ├── TextureResource.h
│           ├── MeshResource.h
│           ├── MaterialResource.h
│           ├── EffectResource.h
│           └── TerrainResource.h
├── src/
│   ├── ResourceManager.cpp
│   ├── Resource.cpp
│   ├── ResourceId.cpp
│   └── ... (实现体)
├── tests/
│   ├── unit/
│   │   ├── test_resource_cache.cpp
│   │   ├── test_load_sync.cpp
│   │   └── ...
│   └── integration/
└── specs/
    └── _contracts/
```

**Structure Decision**: 单模块库；公开头文件与 ABI 表一致；029-World 的 IModelResource 在 te/world 下由 029 实现，013 仅通过 IResource* 返回并可由调用方转型。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> 下表仅列出**相对于现有 `specs/_contracts/013-resource-ABI.md` 的新增与修改**。写回时将这些条目增补到 ABI 文件中。实现时使用**全量 ABI 内容**（见上方「全量 ABI 内容（实现参考）」）。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| 新增 | 013-Resource | te::resource | IResourceManager | 缓存查询 | te/resource/ResourceManager.h | IResourceManager::GetCached | `IResource* GetCached(ResourceId id) const;` 仅查缓存，未命中返回 nullptr |
| 新增 | 013-Resource | te::resource | IResourceManager | 注册反序列化器 | te/resource/ResourceManager.h | IResourceManager::RegisterDeserializer | `void RegisterDeserializer(ResourceType type, IDeserializer* deserializer);` |
| 新增 | 013-Resource | te::resource | IResourceManager | 注册 Importer | te/resource/ResourceManager.h | IResourceManager::RegisterImporter | `void RegisterImporter(ResourceType type, IResourceImporter* importer);` |
| 新增 | 013-Resource | te::resource | IResourceManager | Import | te/resource/ResourceManager.h | IResourceManager::Import | `bool Import(char const* path, ResourceType type, void* out_metadata_or_null);` |
| 新增 | 013-Resource | te::resource | IResourceManager | Save | te/resource/ResourceManager.h | IResourceManager::Save | `bool Save(IResource* resource, char const* path);` |
| 新增 | 013-Resource | te::resource | IResourceManager | 寻址解析 | te/resource/ResourceManager.h | IResourceManager::ResolvePath | `char const* ResolvePath(ResourceId id) const;` 或等价 |
| 新增 | 013-Resource | te::resource | IResourceLoader | Loader 接口 | te/resource/ResourceLoader.h | IResourceLoader::CreateFromPayload | `IResource* CreateFromPayload(ResourceType type, void* payload, IResourceManager* manager);` |
| 新增 | 013-Resource | te::resource | IResourceImporter | Importer 接口 | te/resource/ResourceImporter.h | IResourceImporter | DetectFormat、Convert、产出描述/数据、Metadata、Dependencies |
| 新增 | 013-Resource | te::resource | IDeserializer | 反序列化器接口 | te/resource/Deserializer.h | IDeserializer::Deserialize | `void* Deserialize(void const* buffer, size_t size);` |

## Complexity Tracking

（当前无 Constitution 违反需豁免。）
