# Feature Specification: 013-Resource 完整模块实现

**Feature Branch**: `013-resource-fullmodule-001`  
**Created**: 2026-02-05  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见下方规约引用，契约见 `specs/_contracts/013-resource-public-api.md`；**本 feature 实现完整模块内容**，且需要考虑 public-api 文件中的 **TODO 内容**，参考 **ABI 文件**（`specs/_contracts/013-resource-ABI.md`）中现有接口结构。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/013-resource.md`（013-Resource 模块：资源统一基类 IResource、唯一加载入口、资源缓存、加载工具；导入/序列化/Save/Load 统一接口由各模块实现；依赖 001-Core、002-Object、028-Texture）。
- **对外 API 契约**：`specs/_contracts/013-resource-public-api.md`。
- **本模块范围**（本 feature 实现完整模块内容；对应契约能力与 public-api TODO）：
  1. **IResource 基类与类型**：IResource、GetResourceType、Release；ResourceType 枚举（见 ABI：`te/resource/ResourceTypes.h`）；类型化视图由各模块定义，013 返回 IResource*。
  2. **统一加载**：RequestLoadAsync、LoadSync 为唯一入口；按 ResourceType 分发；通过注册调用 010/011/012/028/029 的 Create*/Loader；Load 阶段不创建 DResource；实现 public-api TODO「Load 入口」「Load 与注册」「不透明 payload 传递」。
  3. **资源缓存**：按 ResourceId 缓存 IResource*；GetCached(ResourceId)；与 Unload/GC 协调（TODO：缓存、Unload、依赖加载、状态与回调）。
  4. **加载工具**：GetLoadStatus、GetLoadProgress、CancelLoad；RequestStreaming、SetStreamingPriority；RegisterResourceLoader（见 ABI：IResourceManager 各接口）。
  5. **寻址**：ResourceId、GUID、可寻址路径、BundleMapping；ResolvePath(ResourceId) 或等价（TODO：Addressing）。
  6. **卸载**：Unload(IResource*)、IResource::Release()；与各模块句柄协调；TODO：Unload、EnsureDeviceResources 转发。
  7. **EnsureDeviceResources**：EnsureDeviceResourcesAsync/EnsureDeviceResources 由下游触发并转发给具体 IResource 实现；013 不参与 DResource 创建。
  8. **导入（统一接口）**：RegisterImporter(ResourceType, IResourceImporter*)、Import(path, type)；各模块实现 IResourceImporter；TODO：导入统一接口。
  9. **序列化（统一接口）**：Serialize/Deserialize 统一入口或与 002 按类型分发；各模块实现本类型序列化/反序列化；TODO：序列化统一接口。
  10. **Save（统一接口）**：Save(IResource*, path)；存盘时各模块从 IResource 返回内存内容，013 调用统一写接口落盘；TODO：Save 流程。
  11. **Load（统一接口）**：RequestLoadAsync/LoadSync 为唯一入口；Loader 接收不透明 payload；各模块实现 IResourceLoader；见 TODO：Load 与注册、接口。

实现时只使用**本模块依赖的上游契约**中已声明的类型与 API：
- **001-Core**：`specs/_contracts/001-core-public-api.md`
- **002-Object**：`specs/_contracts/002-object-public-api.md`
- **028-Texture**：`specs/_contracts/028-texture-public-api.md`（013 依赖 Texture，见 `specs/_contracts/000-module-dependency-map.md`）

不实现上游契约未声明的类型或 API。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现 **`specs/_contracts/013-resource-ABI.md`** 中的**全部**符号与能力；接口形态与命名空间、头文件、签名以 ABI 表为准；构建须通过引入真实子模块源码满足依赖，禁止长期使用 stub/mock。契约/ABI 更新：接口变更须在 013-resource-ABI.md 中增补或替换对应条目；下游所需接口须在上游模块 ABI 中以 TODO 登记。详见 `specs/_contracts/README.md`。

- **第三方依赖**：在契约 `specs/_contracts/013-resource-public-api.md` 中声明；本 spec 引用该契约即可，不重复列出。

## Clarifications

### Session 2026-02-05

- Q: 同一 ResourceId 多次 Load（未 Release）时，013 的约定行为？ → A: 同一 ResourceId 多次 Load 返回同一 IResource*；首次加载创建并入缓存，后续命中缓存并增加引用计数（或同一句柄多引用）；Release 次数须与「获取」次数匹配才能完全释放。
- Q: GetCached(ResourceId) 未命中时的约定行为？ → A: 仅查询缓存；未命中时只返回 nullptr（或等价「未找到」），不触发加载；加载必须通过 RequestLoadAsync/LoadSync 进行。
- Q: 013 在 Load 流程中反序列化（buffer → payload）的约定方式？ → A: 013 按 ResourceType 调用各模块注册的反序列化器；013 读文件得 buffer 后按 ResourceType 调该类型反序列化接口得到 opaque payload，再传给 Loader；不要求 013 直接调 002 按 TypeId 反序列化；各模块可在其反序列化器内部使用 002。
- Q: 资源依赖图存在循环引用时 013 的约定策略？ → A: 禁止；检测到循环引用时视为错误，加载失败并返回错误（如 LoadResult::Error 或等价）；依赖图必须无环。
- Q: 异步加载完成时对外暴露的回调/状态约定？ → A: 仅「已加载」；RequestLoadAsync 的 LoadCompleteCallback 在根资源及其递归依赖均加载完成后调用一次，传入的 IResource* 即可用；不单独提供「根已创建」回调；GetLoadStatus 可区分 Pending/Loading/Completed/Failed/Cancelled。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 通过统一接口按 ResourceType 加载资源 (Priority: P1)

调用方需要加载任意类型资源（Texture、Mesh、Material、Model 等）；系统提供唯一入口 RequestLoadAsync(path, type, callback) 或 LoadSync(path, type)，按 ResourceType 分发到已注册的 Loader，返回 IResource* 并纳入缓存；调用方通过 GetResourceType() 与类型化接口使用。

**Why this priority**: 统一加载是 013 的核心职责，下游（Pipeline、Audio、Editor 等）均依赖此能力。

**Independent Test**: 给定合法路径与 ResourceType，调用 LoadSync 或 RequestLoadAsync，在依赖满足时得到有效 IResource*；可通过 GetCached(ResourceId) 再次获取；不依赖具体资源格式实现。

**Acceptance Scenarios**:

1. **Given** Core、Object、028-Texture 已初始化且对应类型 Loader 已注册，**When** 调用方传入合法 path 与 ResourceType 并调用 LoadSync，**Then** 返回有效 IResource*，且可通过 GetCached(ResourceId) 命中。
2. **Given** 调用 RequestLoadAsync，**When** 根资源及其递归依赖均加载完成，**Then** LoadCompleteCallback 被调用一次，传入的 IResource* 即可用；GetLoadStatus/GetLoadProgress 可查询状态与进度（Pending/Loading/Completed/Failed/Cancelled）。
3. **Given** 无效 path 或未注册类型，**When** 调用 LoadSync 或 RequestLoadAsync，**Then** 以约定方式失败（如返回 nullptr、LoadResult::NotFound/Error），不崩溃。
4. **Given** 同一 ResourceId 已通过 LoadSync 加载，**When** 再次调用 LoadSync（同一 ResourceId），**Then** 返回同一 IResource*（引用计数增加）；调用方须对每次获取调用一次 Release，Release 次数与获取次数匹配后资源才完全释放。

---

### User Story 2 - 资源缓存与寻址 (Priority: P2)

调用方持有 ResourceId/GUID，需要查询是否已加载或触发按需加载；系统按 ResourceId 缓存 IResource*，提供 GetCached(ResourceId)；寻址提供 ResolvePath(ResourceId) 或等价，支持多内容根与 Bundle。

**Why this priority**: 缓存与寻址是避免重复加载与可寻址打包的前提。

**Independent Test**: 加载后以 ResourceId 调用 GetCached 可得到同一 IResource*；ResolvePath(ResourceId) 返回可读路径或包内位置（与实现约定一致）。

**Acceptance Scenarios**:

1. **Given** 某资源已通过 LoadSync/RequestLoadAsync 加载，**When** 使用其 ResourceId 调用 GetCached，**Then** 返回同一 IResource*。
2. **Given** ResourceId 未在缓存中（未加载或已释放），**When** 调用 GetCached，**Then** 返回 nullptr（或等价「未找到」），且不触发加载；加载须通过 RequestLoadAsync/LoadSync 进行。
3. **Given** 合法 ResourceId，**When** 调用 ResolvePath（或等价），**Then** 返回可解析的路径或包内引用，与 013 的 Addressing 约定一致。

---

### User Story 3 - 卸载、释放与生命周期 (Priority: P2)

调用方在不再需要资源时调用 Unload(IResource*) 或 IResource::Release()；系统更新引用计数或执行卸载，与各模块句柄协调，DResource 随 RResource 由各子类/028/011/012 销毁，013 不直接操作 008。

**Why this priority**: 释放与卸载是生命周期闭环与无泄漏的保证。

**Independent Test**: LoadSync 得到 IResource* 后调用 Release/Unload，资源可回收；对同一句柄多次 Release 为幂等；与下游模块句柄释放顺序协调。

**Acceptance Scenarios**:

1. **Given** 已获得的 IResource*，**When** 调用 IResource::Release() 或 IResourceManager::Unload，**Then** 句柄失效，资源可被回收，无悬空引用。
2. **Given** 对已释放或无效句柄再次调用 Release/Unload，**When** 调用，**Then** 行为幂等，不崩溃。

---

### User Story 4 - 导入、序列化、Save 统一接口 (Priority: P3)

调用方或管线需要导入外部资产、序列化/反序列化、存盘；013 提供统一接口 RegisterImporter、Import、Serialize/Deserialize、Save(IResource*, path)；各模块实现对应类型的 Importer/Loader 与「IResource → 内存内容」；013 负责「内存内容 → 磁盘」写盘，不解析 *Desc 内容。

**Why this priority**: 完整模块需覆盖导入与存盘流程，便于编辑器和管线使用。

**Independent Test**: 对已加载的 IResource* 调用 Save，013 按 GetResourceType() 分发到对应模块取得内存内容并写盘；Import(path, type) 按 type 分发到已注册 Importer；反序列化产出不透明 payload 并传入 Loader。

**Acceptance Scenarios**:

1. **Given** 有效 IResource* 与路径，**When** 调用 Save(IResource*, path)，**Then** 各模块返回可存盘内容，013 调用统一写接口落盘，不直接写文件于各模块。
2. **Given** 外部资产路径与 ResourceType，**When** 调用 Import(path, type)，**Then** 013 按 type 分发到已注册 IResourceImporter，产出描述/数据与元数据。

---

### Edge Cases

- 同一 ResourceId 多次 LoadSync/RequestLoadAsync 未 Release：**返回同一 IResource***；首次加载创建并入缓存，后续调用命中缓存并增加引用计数；调用方每次 Release 与每次「获取」对应，Release 次数与获取次数匹配后资源才完全释放。
- Load 期间依赖的上游（Core 文件、Object 反序列化、028 CreateTexture）不可用：以约定方式失败并清理，不泄漏句柄。
- 循环引用：**禁止**；依赖加载时通过 013 统一 Load 递归，若检测到循环引用则加载失败并返回错误（如 LoadResult::Error）；依赖图必须无环。
- 异步加载未完成时调用 GetCached：若该 ResourceId 尚未入缓存则返回 nullptr（GetCached 不触发加载）；GetLoadStatus/GetLoadProgress 可查询进行中请求的状态与进度。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统 MUST 实现 `specs/_contracts/013-resource-public-api.md` 中全部 11 项能力汇总；RequestLoadAsync 的 LoadCompleteCallback 仅在根及其递归依赖均加载完成（「已加载」）时调用一次，不单独提供「根已创建」回调。（IResource 基类与类型、统一加载、资源缓存、加载工具、寻址、卸载、EnsureDeviceResources、导入/序列化/Save/Load 统一接口），并满足该文件中 **TODO 列表** 的约定（Load 入口、缓存、依赖加载、状态与回调、Unload、Streaming、EnsureDeviceResources、导入/序列化/Save 流程、Load 与注册、接口签名）。
- **FR-002**: 系统 MUST 实现 `specs/_contracts/013-resource-ABI.md` 中 ABI 表的**全部**符号与接口结构（命名空间、头文件、签名与说明）；新增或变更接口须在 ABI 文件中增补或替换对应条目。
- **FR-003**: 实现 MUST 仅使用 **001-Core**（`specs/_contracts/001-core-public-api.md`）、**002-Object**（`specs/_contracts/002-object-public-api.md`）、**028-Texture**（`specs/_contracts/028-texture-public-api.md`）契约中已声明的类型与 API；不引入未声明依赖。
- **FR-004**: Load 流程 MUST 采用「不透明 payload」传递：013 读文件得 buffer → **按 ResourceType 调用各模块注册的反序列化器**得到 opaque payload → 按 ResourceType 调用已注册 Loader 并传入 payload；013 不解析各模块 *Desc、不要求直接调 002 按 TypeId 反序列化；Loader 由拥有 *Desc 的模块实现。
- **FR-005**: Save 流程 MUST 为「各模块从 IResource 产出内存内容 → 013 调用统一写接口落盘」；各模块不直接写文件。
- **FR-006**: 013 MUST 不创建、不持有、不调用 008-RHI；DResource 由 011/012/028 在 EnsureDeviceResources 时创建，013 仅转发调用。
- **FR-007**: 资源依赖图 MUST 无环；检测到循环引用时加载失败并返回错误（如 LoadResult::Error 或等价）。

### Key Entities

- **IResource**：可加载资源统一基类；GetResourceType()、Release()；013 缓存与返回均为 IResource*。
- **ResourceType**：资源类型枚举；定义于 013，见 ABI `te/resource/ResourceTypes.h`。
- **ResourceId / GUID**：资源全局唯一标识；缓存键、寻址、与 Object 引用解析对接。
- **IResourceManager**：统一加载、缓存、卸载、加载工具入口；RequestLoadAsync、LoadSync、GetCached、Unload、GetLoadStatus、GetLoadProgress、CancelLoad、RequestStreaming、SetStreamingPriority、RegisterResourceLoader。
- **LoadRequestId、LoadStatus、LoadResult、LoadCompleteCallback**：异步加载请求与回调；见 ABI。
- **FResource / RResource / DResource**：资源三态（概念）；013 仅创建 RResource，不创建 DResource。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 调用方在给定合法 path 与 ResourceType 且上游与 Loader 可用时，能通过 RequestLoadAsync 或 LoadSync 得到有效 IResource*，并可通过 GetCached(ResourceId) 再次获取。
- **SC-002**: 调用方对 IResource* 调用 Release/Unload 后，资源可回收且无悬空引用；与各模块句柄释放顺序协调。
- **SC-003**: 本模块实现可通过 ABI 表中全部符号的编译与链接验收；行为满足契约能力与 TODO 约定。
- **SC-004**: 导入、序列化、Save、Load 均通过 013 统一接口完成；Save 时各模块只返回内存内容，013 负责写盘；Load 时 013 仅传递不透明 payload 给 Loader。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/013-resource-public-api.md`（完整实现全部能力与 TODO）。
- **本模块依赖的契约**：见下方 Dependencies。
- **ABI/构建**：须实现 `specs/_contracts/013-resource-ABI.md` 中全部符号；构建须引入真实子模块代码，禁止长期使用 stub。接口变更须在 013-resource-ABI.md 中更新；下游所需接口须在上游 ABI 中以 TODO 登记。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`（文件、内存、平台 I/O、异步等）；实现时只使用契约已声明的类型与 API。
- **002-Object**：`specs/_contracts/002-object-public-api.md`（序列化、反射、GUID/引用解析、*Desc 注册）；实现时只使用契约已声明的类型与 API。
- **028-Texture**：`specs/_contracts/028-texture-public-api.md`（013 依赖 Texture；CreateTexture 等由 028 实现，013 在 Load(Texture) 时调用）；实现时只使用契约已声明的类型与 API。
- 依赖关系总览：`specs/_contracts/000-module-dependency-map.md`。
