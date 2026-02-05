# 2. 资源加载流程与引用关系

本文档描述引擎资源的**加载流程**与**引用关系**，包含 GUID 与文件路径的对应、同步/异步加载、依赖解析、EnsureDeviceResources、卸载与流式。**所有“如何加载”的约定与流程以本文档为准**；01-asset-data-structure 仅描述数据结构与磁盘/内存形态。

---

## 2.1 设计原则（与文档 1 一致）

| # | 原则 |
|---|------|
| 1 | 所有资源依 **GUID** 管理；磁盘描述仅 GUID 引用，RResource 间指针引用。 |
| 2 | 所有资源继承 **IResource**，统一加载、卸载、导入接口。 |
| 3 | 磁盘：一目录一资源（描述 + 实际数据 + 导入前源数据）。 |
| 4 | 内存：使用方仅持 ResourceId/句柄，经 Resource 层解析；RResource 间指针层级引用。 |
| 5 | **IResource 子类**提供 **EnsureDeviceResources**，在需要渲染时内部调用 008-RHI（或经 028/011/012）创建 DResource。 |

---

## 2.2 GUID 与文件路径的对应

加载时须根据 **GUID（ResourceId）** 定位到磁盘上的资源目录或包内位置，才能读入描述文件与实际数据。

### 2.2.1 约定要求

- 每个 GUID **唯一对应**一份资源数据所在位置。
- 该位置须可读入：**资源描述文件**（.material / .mesh / .texture / .model / .level / .shader 等）及该资源所需的**实际数据文件**（如 .texdata、.meshdata）。
- 与 01 一致：**一目录一资源**，即一个 GUID 对应一个资源目录（或包内等价结构）。

### 2.2.2 实现方式：注册表

**采用注册表（Manifest / 资源注册表）**：维护 **GUID → 路径** 的映射；013 启动时或按需加载该注册表，加载资源时根据 GUID **查表**得到路径再读文件。路径可为文件系统路径或包内路径（含包名 + 包内偏移/路径）。

- **多内容根**：可支持多个根（引擎/项目/DLC）；GUID 全局唯一，注册表中可带根标识，解析时先定根再得完整路径。
- **Bundle**：打包后 GUID 对应包内路径/偏移；打包工具将「GUID → 包内位置」写入注册表或包头，013 Addressing 查表后在对应包内读。

### 2.2.3 解析接口（概念）

- Resource 层（013）提供 **GUID → 可读路径/句柄**：如 `ResolvePath(ResourceId)` 或内部等价，**通过查注册表**得到用于读描述与实际数据的路径或包内读句柄。

---

## 2.3 统一加载流程（所有资源类型一致）

所有资源类型均按同一套流程加载，差异仅在于 *AssetDesc 类型与具体 IResource 实现归属的模块。**依赖的加载由 IResource 实现在创建时递归完成**，不在 Resource 层先统一递归。

1. **入口**：调用方向 Resource 层提供 **GUID（ResourceId）**。
2. **解析路径**：Resource 层按 §2.2（注册表）将 GUID 解析为资源路径（或包内位置）。
3. **读盘**：读入**资源描述文件**及该资源所需的**实际数据文件**（如 .texdata、.meshdata）。
4. **反序列化**：Resource 层将描述与原始数据交 **002-Object**，由 **002 统一反序列化**得到 *AssetDesc 及实际数据在内存中的表示。
5. **创建 RResource**：Resource 层将 ***AssetDesc 与实际数据** 交给**该资源类型对应的 IResource 实现**（各模块向 013 注册的 Create*/Loader）。由该实现在**创建过程中**通过 Resource 层统一 Load 接口**递归加载并创建依赖**（依赖未在缓存则触发加载），最终完成**根 RResource** 的创建。
6. **缓存与返回**：**创建完成根 RResource 后即**将 IResource* 按 ResourceId 缓存，并**立即**对调用方返回 **ResourceId/句柄**（不等待该根下递归依赖全部加载完成才返回）。
7. **Load 阶段不创建 DResource**。

**RResource 加载状态**：RResource 提供可查询的**加载状态**（如 `Loading` / `Ready`）。仅当该 RResource **递归依赖的所有资源**全部加载完成后，状态才标注为**已加载（Ready）**。**异步加载**时，返回句柄后根 RResource 可能仍为“加载中”，上层须通过**状态检查**（如 `GetLoadState()`）或**已加载回调**（递归依赖全部完成时触发）等待**已加载**后再使用；详见 §2.6.1。同步加载时，若 IResource 实现在创建根时同步等待依赖，则返回时通常已为已加载。

Level 资源：描述与数据归属 029-World；029 完成 Level 的 RResource 创建后，可使用 004-Scene 提供的场景管理接口。004 不负责资源，不持有 *AssetDesc。

---

## 2.4 加载入口与调用方

| 调用方 | 行为 |
|--------|------|
| **020-Pipeline** | 从 Scene/Entity 取 ResourceId 或句柄 → 经 013 LoadSync/GetCached 解析；不长期持有 IResource*；LOD 流式经 013。 |
| **029-World** | 经 013 Load(levelGuid) 获取 Level；持有关卡句柄与 SceneRef；使用 004 提供的场景管理。 |
| **016-Audio** | 经 013 Load 获取音频资源；仅持 ResourceId/句柄。 |
| **024-Editor / 022-2D / 023-Terrain** | 调用 013 Load/GetCached；仅持 ResourceId/句柄。 |
| **010/011/012/028** | 不发起加载；013 加载后传入 *AssetDesc、实际数据与已加载依赖，各模块 Create*/Loader 创建 RResource。 |
| **004-Scene** | 不负责资源；由 029 等依赖，提供场景结构、场景管理、数据管理。 |

---

## 2.5 同步加载（LoadSync）

1. 调用方调用 `LoadSync(ResourceId)`。
2. 013 查缓存；命中则直接返回句柄。
3. 按 §2.3 执行：解析路径（注册表）→ 读描述与实际数据 → 002 反序列化 → 交 IResource 实现创建（**实现内部通过统一 Load 接口递归加载依赖**）→ 创建完根 RResource 后即缓存并返回句柄。
4. 返回时根 RResource 已创建并返回；若 IResource 实现在创建根时同步等待依赖，则返回时状态通常已为**已加载**；否则调用方可通过状态查询等待已加载。不创建 DResource。

---

## 2.6 异步加载（LoadAsync）

1. 调用方调用 `LoadAsync(ResourceId, callback?)`，获得 **LoadHandle** 或 **AsyncResult**。
2. 缓存命中时可立即返回句柄或回调。
3. 否则加入加载队列（可设优先级）；后台或下一帧执行 §2.3：解析路径、读盘、002 反序列化 → 交 IResource 实现创建（**实现内部通过统一 Load 接口递归加载依赖**）→ **创建完根 RResource 后即**写缓存并**立即**向调用方返回句柄（或触发**根已创建**回调）（**不等待递归依赖全部完成**）。
4. **上层须等待「递归依赖全部完成」后再使用资源**：根返回后可能仍为**加载中**；仅当该根 **递归依赖的所有资源** 全部加载完成，状态才为 **已加载**。上层通过 **状态检查** 或 **已加载回调** 获知此时机（见 §2.6.1）。
5. LoadHandle/AsyncResult 支持 **IsCompleted**、**GetResult**、**Cancel**。

### 2.6.1 上层异步加载的回调与状态检查（含递归依赖完成）

异步加载存在两个完成时机，上层需区分并据此使用资源：

| 时机 | 含义 | 上层可获得 |
|------|------|------------|
| **根已创建** | 根 RResource 已创建并写入缓存，句柄可立即返回或通过回调交付 | 句柄 / ResourceId；此时该资源可能仍为 **加载中**（递归依赖未全完成） |
| **已加载** | 该 RResource **递归依赖的所有资源** 全部加载完成，状态变为 **已加载** | 可安全使用该资源（含其依赖链） |

**回调设计（建议）**：

- **根已创建回调**（可选）：创建完根 RResource 后即调用，参数为 ResourceId/句柄。上层可先拿到句柄做占位、排队等，但**不得**依赖资源内容做逻辑，直至收到「已加载」通知。
- **已加载回调**（推荐提供）：当该 RResource 的**递归依赖全部加载完成**、状态由「加载中」变为「已加载」时调用，参数为 ResourceId/句柄（或 LoadHandle）。上层在**此回调中**或之后方可安全使用该资源（访问内容、提交绘制等）。  
  - 实现要点：Resource 层或 IResource 实现需在**递归依赖树**全部就绪时（自底向上标记或计数）触发该回调，而非仅根创建完成时触发。

**状态检查**：

- **LoadState**：RResource（或通过句柄/ResourceId 查询）提供可查询的 **加载状态**，例如：`Loading`（加载中）、`Ready`（已加载）、可选 `Failed`。
- **GetLoadState(handle / ResourceId)**：上层可**轮询**该接口，当返回 **Ready** 时表示递归依赖已全部加载完成，可安全使用。
- **与回调配合**：上层可仅依赖「已加载」回调，或在拿到句柄后轮询 `GetLoadState()` 直到 `Ready`，再执行业务逻辑。

**推荐用法**：

- 若上层逻辑必须在「资源及其递归依赖全部就绪」后执行（如渲染、关卡进入），应：
  - 使用 **LoadAsync** 的 **已加载回调**（若 API 支持），在该回调内执行业务；或
  - 在收到句柄后**轮询 GetLoadState()** 直至 **Ready**，再执行业务。
- 不推荐在仅收到句柄或「根已创建」回调后即访问资源内容，因此时递归依赖可能尚未加载完成。

---

## 2.7 依赖解析与层级引用

- **依赖由 IResource 实现在创建时递归处理**：描述文件中仅包含**依赖资源的 GUID**；013 将 *AssetDesc 与实际数据交给 IResource 实现后，由该实现在创建过程中通过 Resource 层**统一 Load 接口**（LoadSync/LoadAsync）递归加载依赖并创建子 RResource，不由 013 在外部先统一递归。
- **循环引用**：规范或实现中约定（禁止 / 延迟 / 弱引用），避免死锁或无限递归。
- **典型引用链**：Level → Model(GUID) → Mesh(GUID) + Material(GUID)；Material → Shader(GUID) + Texture(GUID)；Model → 多个 Mesh/Material(GUID) + submesh 材质索引。
- **内存中**：RResource 内持有对其它 RResource 的**指针**（由 IResource 实现在创建时填入）；卸载时与引用计数或 GC 协调，避免悬空指针。

---

## 2.8 EnsureDeviceResources（GPU 资源创建）

- **时机**：Load 阶段不创建 DResource；由**下游**（如 020-Pipeline）在需要提交绘制前，对 ResourceId/句柄调用 **EnsureDeviceResources** 或 **EnsureDeviceResourcesAsync**。
- **执行方**：**IResource 子类**实现 EnsureDeviceResources；子类内部**调用 008-RHI**（或经 028/011/012 封装）创建 DResource。013 不创建、不调用 008。
- **接口**：IResource 暴露 EnsureDeviceResources / EnsureDeviceResourcesAsync；013 将调用转发给具体 RResource。
- **层级**：若 RResource 依赖其它 RResource（如 Material 依赖 Texture），先对依赖链上的资源 EnsureDeviceResources，再对当前资源创建 DResource。

---

## 2.9 卸载与释放

- **Release**：调用方对句柄/ResourceId 调用 Release；013 递减引用或从使用集合移除；无引用时可卸载 RResource。
- **Unload / GC**：013 按 UnloadPolicy、引用计数或 GC 策略卸载；与各模块句柄协调，避免悬空引用。
- **DResource**：随 RResource 卸载，由 RResource 内部或 028/011/012/008 销毁；013 不直接操作。

---

## 2.10 流式与按需加载

- 013 提供 **RequestStreaming**、**SetPriority** 等（StreamingHandle），与 LOD、地形等按需加载对接。
- 可先加载描述或低精度数据，高精度块按需 LoadAsync；依赖仍以 GUID 表达，013 统一解析与回调。

---

## 2.11 调用顺序与约束

- 须在 **001-Core、002-Object、028-Texture** 初始化后使用 013；**010/011/012/028/029** 须已向 013 **注册** Create*/Loader。
- 资源引用（GUID / ObjectRef）格式须与 002 序列化约定一致。
- 013 不创建、不持有、不调用 008；DResource 由 IResource 子类在 EnsureDeviceResources 时调 008（或 028/011/012）创建。
- 句柄释放顺序须与 Resource 卸载策略协调。

以上与 `docs/module-specs/013-resource.md`、`specs/_contracts/013-resource-public-api.md`、`000-module-dependency-map.md` 及 028/029 契约一致。
