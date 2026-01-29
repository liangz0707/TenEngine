# Feature Specification: 004-Scene 完整功能集

**Feature Branch**: `004-scene-fullversion-001`  
**Created**: 2026-01-29  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见下方规约与契约引用；本 feature 需实现完整功能集（见下方本切片范围）。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/004-scene.md`（场景/关卡与层次结构：场景图、层级、World/Level、激活/禁用；依赖 Core、Object）。
- **对外 API 契约**：`specs/_contracts/004-scene-public-api.md`。
- **本切片范围（完整功能集，显式枚举）**：
  1. **SceneGraph**：Node、Parent/Children、LocalTransform、WorldTransform、SetDirty、UpdateTransforms。
  2. **Hierarchy**：Traverse、FindByName/ByType、GetPath、GetId、SetActive。
  3. **World**：GetCurrentWorld、SetActiveWorld、AddWorld、与 Entity 根实体挂接。
  4. **Level**：LoadLevel、UnloadLevel、Level 资源句柄（与 Resource 配合）。
  5. **激活/禁用**：节点与子树参与更新/渲染的开关；下游可查询 Active 状态。

实现时只使用 `specs/_contracts/001-core-public-api.md`、`specs/_contracts/002-object-public-api.md` 及与本 feature 约定的 013-Resource 接口中已声明的类型与 API；对外行为以 `specs/_contracts/004-scene-public-api.md` 为准。

## Clarifications

### Session 2026-01-29

- Q: 本切片内 HierarchyIterator 的生命周期应采纳哪一种（单次有效 / 由调用方管理 / 两者都支持）？ → A: 单次有效（Traverse/Find 返回的迭代器在本次遍历结束后即失效，不可复用）。
- Q: 本切片内 LoadLevel/UnloadLevel 与 013-Resource 的调用与失败语义应在哪一层约定？ → A: 本 spec 不规定；由 plan 与 013-Resource 对接时约定句柄、加载时机与失败语义。
- Q: 本切片内 SetActiveWorld 调用后，「当前活动 World」应在何时生效？ → A: 下一帧或下一次 UpdateTransforms 生效；本帧内仍为旧 World，之后为新 World。
- Q: 本切片内父子成环或非法层级（如将节点设为自己的父节点）应如何处理？ → A: 拒绝并返回错误；API 返回错误码/结果，不修改场景图，调用方可恢复。
- Q: 本切片内 Entity 根挂接的接口归属（Scene 提供 API 还是 Entity 提供 API）应在哪一层约定？ → A: 本 spec 不规定；由 plan 与 005-Entity 对接时约定接口归属与签名。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 场景图与变换层级 (Priority: P1)

作为引擎或工具开发者，我需要在 World 下构建场景图、设置父子关系与局部/世界变换，并通过脏标记与 UpdateTransforms 保持变换一致性。

**Why this priority**: 场景图与变换是层级、World/Level 与激活/禁用的基础。

**Independent Test**: 创建 World、创建多级节点、设置 LocalTransform、SetDirty 后 UpdateTransforms，验证 WorldTransform 与层级一致；下游可依赖 NodeId、Transform。

**Acceptance Scenarios**:

1. **Given** Core 与 Object 已初始化，**When** 获取或创建 World、创建根节点与子节点并设置 Parent/Children 与 LocalTransform，**Then** 可得到稳定 WorldRef、NodeId，且父子关系可查询。
2. **Given** 某节点 LocalTransform 被修改，**When** 调用 SetDirty 后执行 UpdateTransforms，**Then** 该节点及其子节点的 WorldTransform 与层级一致。
3. **Given** 多 World 存在，**When** 调用 GetCurrentWorld 或 SetActiveWorld，**Then** 当前活动 World 语义明确，变换更新顺序与脏标记一致。

---

### User Story 2 - 层级遍历与按名/按类型查找 (Priority: P2)

作为下游模块（如 Entity、Editor），我需要对场景图进行层级遍历、按名或按类型查找节点，并获取路径与 ID、设置激活/禁用。

**Why this priority**: 层级与激活/禁用是 Entity 挂接与渲染/更新开关的前提。

**Independent Test**: 在已有场景图上执行 Traverse、FindByName/ByType、GetPath、GetId、SetActive，验证迭代器与查询结果、Active 状态与规约一致。

**Acceptance Scenarios**:

1. **Given** 场景图已构建，**When** 使用 Traverse 遍历子树，**Then** 访问顺序与层级一致，返回的 HierarchyIterator 单次有效（遍历结束后即失效）。
2. **Given** 节点具有名称或类型信息，**When** 调用 FindByName 或 FindByType，**Then** 返回符合条件的节点或集合；GetPath、GetId 返回与契约一致的路径与 NodeId。
3. **Given** 某节点或子树，**When** 调用 SetActive 关闭激活，**Then** 下游查询 Active 状态为未激活，该子树不参与更新/渲染（语义由契约约定）。

---

### User Story 3 - 多 World 与当前活动场景 (Priority: P2)

作为应用或关卡逻辑，我需要创建多个 World、切换当前活动 World，并保证当前活动场景的语义明确。

**Why this priority**: 多 World 与活动场景是 Level 加载与 Entity 根挂接的容器。

**Independent Test**: AddWorld、GetCurrentWorld、SetActiveWorld 后，当前 World 与变换更新、层级查询所针对的 World 一致。

**Acceptance Scenarios**:

1. **Given** 已有一个或多个 World，**When** 调用 AddWorld 增加 World、SetActiveWorld 切换当前 World，**Then** 本帧内 GetCurrentWorld 仍返回旧 WorldRef，下一帧或下一次 UpdateTransforms 后返回新 WorldRef。
2. **Given** 当前活动 World 已切换（已过生效点），**When** 执行场景图操作（遍历、变换更新），**Then** 操作针对当前活动 World，与契约「调用顺序与约束」一致。

---

### User Story 4 - Level 加载/卸载与 Resource 对接 (Priority: P3)

作为关卡或资源管线，我需要通过 LoadLevel、UnloadLevel 加载/卸载关卡，并使用 Level 资源句柄与 013-Resource 对接。

**Why this priority**: Level 粒度与加载边界是完整场景/关卡能力的一部分。

**Independent Test**: 在 plan 与 013-Resource 约定句柄与加载时机后，调用 LoadLevel/UnloadLevel，验证关卡加载/卸载边界与 Level 资源句柄生命周期符合约定。

**Acceptance Scenarios**:

1. **Given** Resource 模块可用且已约定关卡句柄/加载时机，**When** 调用 LoadLevel 并传入关卡资源引用，**Then** 关卡加载进当前或指定 World，Level 资源句柄按 Resource 契约管理。
2. **Given** 某 Level 已加载，**When** 调用 UnloadLevel，**Then** 该关卡从场景中卸载，资源与节点生命周期按契约释放，不破坏其他 World/Level。

---

### User Story 5 - 与 Entity 根实体挂接 (Priority: P3)

作为 Entity 或下游模块，我需要在当前（或指定）World 上挂接根实体，使场景与实体层次对接。

**Why this priority**: World 与 Entity 根挂接是规约与契约明确的能力，完成完整功能集需实现。

**Independent Test**: 在 plan 与 005-Entity 约定接口后，在 World 上挂接 Entity 根实体，场景图与实体层次关系一致，下游可基于 SceneRef/WorldRef、NodeId 与 Entity 协作。

**Acceptance Scenarios**:

1. **Given** 当前活动 World 已存在，**When** 在该 World 上挂接 Entity 根实体，**Then** 根实体与场景图根或指定节点关联，下游可查询并使用。
2. **Given** 多 World 存在，**When** 切换 SetActiveWorld，**Then** 各 World 的 Entity 根挂接互不干扰，当前活动 World 的根实体可被下游正确解析。

---

### Edge Cases

- 根节点无 Parent 时，WorldTransform 与 LocalTransform 或 World 根变换的关系；空 World（无节点）时 GetCurrentWorld 仍返回有效 WorldRef。
- 父子成环或非法层级：**拒绝并返回错误**（API 返回错误码/结果，不修改场景图，调用方可恢复）。
- 多 World 切换时，变换更新顺序与脏标记一致性；SetActiveWorld 生效时机为下一帧或下一次 UpdateTransforms；LoadLevel/UnloadLevel 与 Resource 的失败或异步语义由 plan 与 013-Resource 约定。
- HierarchyIterator 单次有效；迭代中修改场景图的行为未定义（调用方不得在单次遍历期间修改该子树结构）。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**：系统 MUST 提供场景图能力：Node、Parent/Children、LocalTransform、WorldTransform、SetDirty、UpdateTransforms；变换更新顺序与脏标记一致。父子成环或非法层级 MUST **拒绝并返回错误**（API 返回错误码/结果，不修改场景图，调用方可恢复）。
- **FR-002**：系统 MUST 提供层级能力：Traverse、FindByName/ByType、GetPath、GetId、SetActive；HierarchyIterator 为**单次有效**（遍历结束后即失效，不可复用），Active 状态可被下游查询。
- **FR-003**：系统 MUST 提供 World 能力：GetCurrentWorld、SetActiveWorld、AddWorld；支持多 World；SetActiveWorld 调用后当前活动 World **下一帧或下一次 UpdateTransforms 生效**（本帧内仍为旧 World）。
- **FR-004**：系统 MUST 提供 Level 能力：LoadLevel、UnloadLevel、Level 资源句柄；句柄、加载时机与失败语义由 plan 与 013-Resource 对接时约定，关卡加载/卸载边界在实现文档中明确。
- **FR-005**：系统 MUST 支持在当前（或指定）World 上与 Entity 根实体挂接，供下游基于 SceneRef/WorldRef、NodeId 与 Entity 协作；接口归属与签名由 plan 与 005-Entity 对接时约定。
- **FR-006**：系统 MUST 在 Core、Object 可用之后使用；多 World 时当前活动场景切换与变换更新、层级查询一致；不实现规约与契约未列出的能力。

### Key Entities

- **SceneRef / WorldRef**：场景/世界容器引用；生命周期为加载后直至卸载。
- **NodeId**：场景图节点 ID；父子关系、层级路径；与节点同生命周期。
- **Transform**：局部/世界变换（与 Core.Math 或共用类型一致）；与节点/实体同步。
- **HierarchyIterator**：层级遍历、按名/按类型查找；迭代器**单次有效**（遍历结束后即失效，不可复用）。
- **Active 状态**：节点/子树是否参与更新/渲染；与节点绑定。
- **Level 资源句柄**：关卡加载/卸载边界；与 Resource 契约一致。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**：在单 World 及多 World 下，构建多级场景图、修改 LocalTransform 后经 SetDirty + UpdateTransforms，所有节点 WorldTransform 与层级一致；下游可稳定使用 NodeId、Transform。
- **SC-002**：Traverse、FindByName/ByType、GetPath、GetId、SetActive 行为与契约一致；Active 状态可被下游正确查询，子树激活/禁用生效。
- **SC-003**：GetCurrentWorld、SetActiveWorld、AddWorld 语义明确；多 World 切换后当前活动 World 与场景图操作一致。
- **SC-004**：LoadLevel/UnloadLevel 与 Resource 对接完成，关卡加载/卸载边界与 Level 资源句柄生命周期符合契约；与 Entity 根挂接可用，下游可基于 SceneRef/WorldRef、NodeId 协作。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：本 feature 完整实现 [specs/_contracts/004-scene-public-api.md](_contracts/004-scene-public-api.md)（类型与句柄、能力列表、调用顺序与约束）。
- **本模块依赖的契约**：见下方 Dependencies。

## Dependencies

- **001-Core**：specs/_contracts/001-core-public-api.md（数学、容器、内存等）。
- **002-Object**：specs/_contracts/002-object-public-api.md（序列化、类型信息）。
- **013-Resource**：关卡/场景资源加载与引用；句柄、加载时机与失败语义由 plan 与 013-Resource 对接时约定（契约为 specs/_contracts/013-resource-public-api.md）。
- **005-Entity**：Entity 根挂接的接口归属与签名由 plan 与 005-Entity 对接时约定（契约为 specs/_contracts/005-entity-public-api.md）。
- 依赖关系总览：specs/_contracts/000-module-dependency-map.md。实现前须从 T0-contracts 拉取最新契约。
