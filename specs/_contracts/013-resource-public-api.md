# 契约：013-Resource 模块对外 API

## 适用模块

- **实现方**：013-Resource（L2；资源统一基类、唯一加载入口、资源缓存与加载工具；不创建 DResource）
- **对应规格**：`docs/module-specs/013-resource.md`
- **依赖**：001-Core、002-Object、028-Texture

## 消费者

- 010-Shader、011-Material、012-Mesh、028-Texture、029-World（向 013 注册 Create*/Loader；029 在 Level 加载时调 004 CreateSceneFromDesc）、016-Audio、020-Pipeline、022-2D、023-Terrain、024-Editor（调用 Load/缓存 等获取资源）

---

## 模块定位与核心职责

013-Resource 的**主要功能**如下，对外 API 围绕这四点展开：

| 职责 | 说明 |
|------|------|
| **IResource 基类与统一接口** | 提供可加载资源的**统一基类 IResource**；所有资源类型（Texture、Mesh、Material、Model 等）均实现或继承该接口；下游通过 IResource* 统一持有、查询类型（GetResourceType）、释放（Release）；类型化视图（ITextureResource、IMeshResource、IModelResource 等）由各模块定义，013 返回 IResource* 后由调用方转型使用。 |
| **统一加载接口** | **唯一加载入口**：RequestLoadAsync(path, type, callback)、LoadSync(path, type)；所有资源类型经同一 API 按 ResourceType 区分；无按类型拆开的多套 Load API。 |
| **资源缓存** | 按 **ResourceId/GUID** 缓存已加载的 **IResource***；提供 GetCached(ResourceId)、按 ID 查找，避免重复加载；缓存生命周期与 Unload/GC 策略一致。 |
| **资源加载工具** | 加载请求管理：LoadRequestId、GetLoadStatus、GetLoadProgress、CancelLoad；流式与优先级：RequestStreaming、SetPriority；寻址：ResourceId、GUID→路径解析、可寻址与 Bundle 映射。**导入/序列化/Save/Load**：013 提供**统一接口**，**各模块实现**对应资源类型的 Import、Serialize/Deserialize、Save/Load（见下文「导入、序列化、Save/Load」）。 |

013 **不创建、不持有、不调用** 008-RHI；Load 阶段仅创建 RResource（内存形态），DResource（GPU 形态）由 011/012/028 在 EnsureDeviceResources 时创建。

---

## 能力列表

### 1. IResource 与统一资源接口

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **IResource** | 可加载资源的**统一基类/接口**；013 返回 IResource*，缓存按 IResource* 存储；提供 GetResourceType()、Release() 等；Model 类型的具体接口 IModelResource 由 029-World 定义 | 013 缓存或下游经 ResourceId/句柄解析持有 |
| **ResourceType** | 资源类型枚举（Texture、Mesh、Material、Model、Effect、Terrain、Shader、Audio 等）；用于请求加载与 IResource::GetResourceType() | 与类型绑定 |
| **ResourceId / GUID** | 资源全局唯一标识；缓存键、FResource 间引用、可寻址路径、与 Object 引用解析对接 | 与资源绑定 |

### 2. 统一加载接口与加载工具

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **IResourceManager** | 统一加载入口与加载工具入口；RequestLoadAsync、LoadSync、GetCached、Unload、GetLoadStatus、GetLoadProgress、CancelLoad、RequestStreaming、SetStreamingPriority、RegisterResourceLoader | 由 Subsystems 或单例提供，调用方不拥有指针 |
| **LoadRequestId** | 异步加载请求句柄；由 RequestLoadAsync 返回 | 请求发出至完成或取消 |
| **LoadStatus / LoadResult** | 加载状态与结果；Pending/Loading/Completed/Failed/Cancelled；供 GetLoadStatus、回调使用 | 与请求或回调绑定 |
| **LoadCompleteCallback** | 异步加载完成回调；参数为 IResource*、LoadResult、user_data | 由调用方或框架管理 |

### 3. 资源缓存

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| **GetCached(ResourceId)** | 按 ResourceId 查询已加载资源，返回 IResource*；命中则直接返回，未命中可返回 nullptr 或触发按需加载（与实现约定一致） | 返回的 IResource* 与缓存一致 |
| **缓存策略** | 缓存存 IResource*，按 ResourceId 查找；Unload/Release/GC 与引用计数协调；与各模块句柄释放顺序一致 | 与 013 实现约定 |

### 4. 资源三态（概念，用于理解数据流）

| 名称 | 语义 | 说明 |
|------|------|------|
| **FResource** | 硬盘形态 | 引用其它资源仅通过 GUID；加载时从磁盘/包读入 |
| **RResource** | 运行时/内存形态 | 即实现 IResource 的对象；013 仅创建 RResource，不创建 DResource；DResource 槽位由 011/012/028/008 在 EnsureDeviceResources 时填充 |
| **DResource** | GPU 形态 | 由 008/011/012/028 在 EnsureDeviceResources 时创建，对 013 不可见，由 RResource 内部持有 |

### 5. 导入、序列化、Save/Load：统一接口 + 各模块实现

013 提供**统一入口与接口**，**各模块**（010/011/012/028/029 等）**实现**对应资源类型的逻辑；013 不实现具体格式，只负责调度与磁盘 I/O。

| 能力 | 013 提供 | 各模块实现 |
|------|----------|------------|
| **Import（导入）** | **统一接口**：RegisterImporter(ResourceType, IResourceImporter*)、Import(path, type) 等；按 type 分发到已注册的 Importer；元数据与依赖记录由 013 或各模块在实现中填充 | 各模块实现 **IResourceImporter**：DetectFormat、Convert、产出引擎格式描述/数据；如 028 实现 Texture 导入、012 实现 Mesh 导入、029 实现 Model 聚合等 |
| **Serialize（序列化）** | **统一接口**：Serialize(ResourceType, payload, buffer)、Deserialize(ResourceType, buffer, out payload) 或通过 002 注册各类型序列化器；013 在 Load/Save 流程中调用 | 各模块实现本类型 ***Desc/内存结构** 的序列化与反序列化（与 002-Object 约定一致）；013 不解析具体 *Desc 内容，只按 type 分发 |
| **Save（存盘）** | **统一接口**：Save(IResource*, path) 或 Save(ResourceId, path)；负责**写入路径解析、打开文件/包、调用统一写接口**将内存内容落盘 | **存盘过程**：013 根据 IResource::GetResourceType() 分发到对应模块；**各模块从 IResource 产出可存盘的内存内容**（序列化后的缓冲或 *Desc + 二进制块），**返回给 013**；013 再调用**统一保存接口**（如 001 文件写或包写入）将内容写入磁盘。即：各模块不直接写文件，只负责“IResource → 内存内容”；013 负责“内存内容 → 磁盘” |
| **Load（加载）** | **统一接口**：RequestLoadAsync、LoadSync 为唯一入口；读文件、按 ResourceType 分发、调用反序列化与各模块 Loader、缓存并返回 IResource* | 各模块实现 **IResourceLoader** / Create*：接收 013 传入的**不透明负载（payload）**，**创建并初始化 IResource 实现体**（RResource），返回 IResource*；依赖资源通过 013 统一 Load 递归加载 |

**小结**：Import、序列化、Save、Load 均为「013 统一接口，各模块按 ResourceType 实现」；Save 时各模块只返回内存内容，由 013 调用统一接口完成写盘。

**Load 时 *Desc 对 013 不可见，如何传递给 IResource 实现？**  
013 **不持有、不解析** ModelAssetDesc、TextureAssetDesc 等各模块 *Desc。**传递方式**：  
1. 013 读文件得到字节缓冲（buffer）。  
2. 反序列化由**统一入口**完成：要么 002 按 TypeId 反序列化（各模块事先向 002 注册 *Desc 类型），要么各模块注册的反序列化器被 013 按 ResourceType 调用；产出为**不透明负载（opaque payload）**（如 `void*` 或 `IDeserializedPayload*`），013 **不解析其内容**，仅按 ResourceType 持有或传递。  
3. 013 按 ResourceType 调用已注册的 **IResourceLoader**，将**该 payload 原样传入** Loader（如 `CreateFromPayload(type, payload, ...)`）。  
4. **Loader 由拥有该 *Desc 的模块实现**（如 029 实现 Model Loader）；Loader 内部将 payload 解释为本模块的 *Desc（如 `(ModelAssetDesc*)payload`），据此创建 IResource 实现体（RResource），返回 IResource*。  

因此：*Desc 仅对拥有模块可见；013 只负责读文件、调用反序列化、拿到 opaque payload、按 type 调用 Loader 并传入 payload，不接触 *Desc 定义。

### 6. 其他跨边界类型（按需）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| StreamingHandle | 流式请求句柄；与 LOD/地形按需加载对接 | 请求有效期内 |
| Metadata | 资源元数据；格式、依赖记录、与导入管线对接 | 与资源或导入产物绑定 |

### 能力汇总（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | **IResource 基类与类型** | 提供 IResource 及 GetResourceType、Release；ResourceType 枚举；类型化视图由各模块定义，013 返回 IResource* |
| 2 | **统一加载** | RequestLoadAsync、LoadSync 为唯一入口；按 ResourceType 分发；通过注册调用 010/011/012/028/029 的 Create*/Loader；Load 阶段不创建 DResource |
| 3 | **资源缓存** | 按 ResourceId 缓存 IResource*；GetCached(ResourceId)；与 Unload/GC 协调 |
| 4 | **加载工具** | GetLoadStatus、GetLoadProgress、CancelLoad；RequestStreaming、SetStreamingPriority；可选 RegisterResourceLoader |
| 5 | **寻址** | ResourceId、GUID、可寻址路径、BundleMapping；GUID→路径解析（ResolvePath 或等价） |
| 6 | **卸载** | Unload(IResource*)、IResource::Release()；与各模块句柄协调 |
| 7 | **EnsureDeviceResources** | EnsureDeviceResourcesAsync/EnsureDeviceResources 由下游触发并转发给具体 IResource 实现；013 不参与 DResource 创建 |
| 8 | **导入（统一接口）** | 013 提供 RegisterImporter(type, IResourceImporter*)、Import(path, type) 等统一接口；各模块实现 IResourceImporter（DetectFormat、Convert、Metadata、Dependencies） |
| 9 | **序列化（统一接口）** | 013 提供 Serialize/Deserialize 统一入口或与 002 配合按类型分发；各模块实现本类型 *Desc/内存结构的序列化与反序列化 |
| 10 | **Save（统一接口）** | 013 提供 Save(IResource*, path) 等统一接口；存盘时各模块从 IResource 返回内存内容，013 调用统一写接口落盘；各模块不直接写文件 |
| 11 | **Load（统一接口）** | RequestLoadAsync/LoadSync 为唯一入口；013 读文件、反序列化后交各模块 Loader/Create* 创建 IResource；各模块实现 IResourceLoader |

*Desc 归属：ModelAssetDesc、IModelResource→029-World；TextureAssetDesc→028-Texture；ShaderAssetDesc→010，MaterialAssetDesc→011，LevelAssetDesc/SceneNodeDesc→029，MeshAssetDesc→012。013 不拥有各模块 *Desc，仅在使用时反序列化或交对应模块组装。*

---

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

---

## 约束

- 须在 Core、Object、028-Texture 初始化之后使用；010/011/012/028/029 须已向 013 注册 Loader（Create*）、可选注册 Importer/序列化器。
- 013 不创建、不持有、不调用 008；资源引用格式须与 Object 序列化约定一致；句柄释放顺序须与卸载策略协调。Import、序列化、Save、Load 均由 013 提供统一接口，各模块实现对应类型的逻辑；Save 时各模块只返回内存内容，013 负责写盘。

---

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **描述归属**：ModelAssetDesc、IModelResource 归属 029-World，TextureAssetDesc 归属 028-Texture；.model/.texture 由 029/028 与 002 注册；013 加载时反序列化得到上述描述并交 029/028 或自身组装；一目录一资源（描述 + 实际数据 + 可选源数据）。
- [ ] **Addressing**：实现 GUID→路径 解析；注册表/Manifest 维护 GUID→路径；支持多内容根、Bundle 包内路径；提供 ResolvePath(ResourceId) 或等价接口。
- [ ] **Load 入口**：LoadSync/LoadAsync 为唯一入口；读文件得 buffer → 统一反序列化得到**不透明 payload**（013 不解析 *Desc）→ 按 ResourceType 调用已注册 Loader，将 payload 原样传入；Loader 由拥有 *Desc 的模块实现，内部将 payload 解释为 *Desc 并创建 IResource；根 RResource 创建完成后即缓存并立即返回句柄（异步不等待递归依赖全部完成）。
- [ ] **缓存**：GetCached(ResourceId) 或等价接口；缓存键为 ResourceId；与 Unload/GC 一致。
- [ ] **依赖加载**：依赖由 IResource 实现在创建时通过 013 统一 Load 接口递归加载；013 不先统一递归；循环引用约定（禁止/延迟/弱引用）并在实现中保证。
- [ ] **状态与回调**：RResource 可查询 LoadState（Loading/Ready/Failed）；提供 GetLoadState(handle/ResourceId)；异步时提供「根已创建」与「已加载」（递归依赖全部完成）回调，供上层安全使用资源。
- [ ] **Unload**：Release、UnloadPolicy、GC 与引用计数；与各模块句柄协调，避免悬空引用；DResource 随 RResource 由各子类/028/011/012/008 销毁，013 不直接操作。
- [ ] **Streaming**：RequestStreaming、SetPriority、StreamingHandle；与 LOD、地形等按需加载对接；可先加载描述或低精度数据，高精度块按需 LoadAsync。
- [ ] **EnsureDeviceResources**：将 EnsureDeviceResources/EnsureDeviceResourcesAsync 转发给具体 RResource，013 不创建、不调用 008。
- [ ] **导入统一接口**：013 提供 RegisterImporter(ResourceType, IResourceImporter*)、Import(path, type)；各模块实现 IResourceImporter（DetectFormat、Convert、产出描述/数据、Metadata、Dependencies）；013 按 type 分发，不实现具体格式。
- [ ] **序列化统一接口**：013 提供 Serialize(type, payload, buffer)/Deserialize(type, buffer, out payload) 或与 002 按类型注册序列化器；各模块实现本类型 *Desc/内存结构的序列化与反序列化；013 在 Load/Save 流程中调用，不解析具体内容。
- [ ] **Save 流程**：013 提供 Save(IResource*, path) 或 Save(ResourceId, path)。存盘时：013 按 IResource::GetResourceType() 分发 → 各模块从 IResource 产出可存盘的内存内容（序列化缓冲或 *Desc+二进制块）并返回 → 013 调用统一写接口（001 FileWrite 或包写入）将内容落盘；各模块不直接写文件。
- [ ] **Load 与注册**：RegisterResourceLoader(type, IResourceLoader*)；各类型 Loader 向 013 注册；Loader 接口约定为接收**不透明 payload**（如 CreateFromPayload(ResourceType, void* payload, ...)），013 不解析 payload 内容；与 002 Serialize、001 FileRead/FileWrite 对接。
- [ ] **接口**：RequestLoadAsync(ResourceId, type, callback)/LoadSync(ResourceId, type)；GetCached(ResourceId)；Save(IResource*, path)；Import(path, type)；EnsureDeviceResourcesAsync(IResource*)；IsDeviceReady(IResource*)/HasDResource(IResource*)；Unload(IResource*)/IResource::Release()。

---

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 013-Resource 契约 |
| 2026-02-05 | 统一目录；能力列表用表格；去除冗余流程说明与 ABI 引用 |
| 2026-02-05 | IModelResource、ModelAssetDesc 归属转移至 029-World；TextureAssetDesc 归属转移至 028-Texture |
| 2026-02-05 | 优化 public-api：突出 IResource 基类、统一加载接口、资源缓存、加载工具四大职责；按职责分组类型与能力 |
| 2026-02-05 | 细化 Import/序列化/Save/Load：013 提供统一接口、各模块实现；Save 存盘流程为各模块返回内存内容、013 调用统一接口保存 |
| 2026-02-05 | 明确 Load 时 *Desc 对 013 不可见：通过不透明 payload 传递；013 读文件→反序列化得 payload→按 type 调用 Loader 并传入 payload，Loader 由拥有 *Desc 的模块实现并解释 payload |
| 2026-02-05 | ABI 写回（plan 013-resource-fullmodule-001）：GetCached、RegisterDeserializer、RegisterImporter、Import、Save、ResolvePath、IResourceLoader::CreateFromPayload、IResourceImporter、IDeserializer::Deserialize 正式纳入 ABI 表 |
