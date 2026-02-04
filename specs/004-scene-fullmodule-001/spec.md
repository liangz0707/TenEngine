# Feature Specification: 004-Scene Full Module

**Feature Branch**: `004-scene-fullmodule-001`  
**Created**: 2026-02-04  
**Status**: Draft  
**Input**: 本 feature 的完整模块规约见 `docs/module-specs/004-scene.md`，契约见 `specs/_contracts/004-scene-public-api.md`；**本 feature 实现完整模块内容**。spec.md 引用规约与契约，描述本模块范围。

## 规约与契约引用 *(模块切片时必填)*

- **完整模块规约**：`docs/module-specs/004-scene.md`（场景/关卡与层次结构：场景图、层级遍历、World/Level、激活/禁用）。
- **本模块范围**（本 feature 实现完整模块内容）：
  1. **SceneGraph**：节点树、父子关系、局部/世界变换、脏标记与变换更新（CreateNode、SetParent、GetParent、GetChildren、GetLocalTransform/SetLocalTransform、GetWorldTransform、SetDirty、UpdateTransforms）。
  2. **Hierarchy**：层级遍历、按名/按类型查找、路径与层级 ID、激活/禁用（Traverse、FindByName、FindByType、GetPath、GetId、SetActive、GetActive；HierarchyIterator 单次有效）。
  3. **World**：场景容器、当前活动场景、多 World（GetCurrentWorld、SetActiveWorld、AddWorld；SetActiveWorld 下一帧或下次 UpdateTransforms 生效）。
  4. **Level**：关卡粒度、加载/卸载边界、与 Resource 的关卡资源引用（LoadLevel、UnloadLevel、LevelHandle；与 013-Resource 约定）。
  5. **激活/禁用**：节点与子树是否参与更新/渲染；与 Entity 根实体挂接（与 005-Entity 约定）。

实现时只使用**本 feature 依赖的上游契约**中已声明的类型与 API：
- **001-Core**：`specs/_contracts/001-core-public-api.md`（数学、容器、内存、日志）。
- **002-Object**：`specs/_contracts/002-object-public-api.md`（序列化、类型信息）。

不实现规约与契约未列出的能力。

- **ABI 与构建规约**（`.specify/memory/constitution.md` §VI）：本模块须实现其 **ABI 文件**（`specs/_contracts/004-scene-ABI.md`）中的**全部**符号与能力；构建须通过**引入真实子模块源码**满足依赖，**禁止**使用 stub、mock 或与契约不一致的代替实现作为长期方案。接口变更须在对应 **ABI 文件**中增补或替换；下游所需接口须在**上游模块的 ABI 文件**中以 **TODO** 登记。详见 `specs/_contracts/README.md`「契约更新流程」。

- **第三方依赖**：在契约 `specs/_contracts/004-scene-public-api.md` 中声明；本 spec 引用该契约即可。

## User Scenarios & Testing *(mandatory)*

### User Story 1 - 场景图与变换层级 (Priority: P1)

在 World 下构建场景图、父子关系、Local/World 变换、SetDirty、UpdateTransforms；仅暴露契约中的场景图 API。

**Why this priority**: 所有层级、World、Level 能力都依赖场景图存在。

**Independent Test**: 创建 World、多级节点、设置 Parent/Children 与 LocalTransform，SetDirty 后 UpdateTransforms，验证 WorldTransform 与层级一致；SetParent 成环/非法层级返回错误。

**Acceptance Scenarios**:

1. **Given** 有效 WorldRef，**When** CreateNode(world)，**Then** 返回有效 NodeId。
2. **Given** 两节点 node、parent，**When** SetParent(node, parent)，**Then** 成功时 GetParent(node)==parent；若成环则返回 false 且场景不变。
3. **Given** 节点已设 LocalTransform 并 SetDirty，**When** UpdateTransforms(world)，**Then** GetWorldTransform 与层级一致。

---

### User Story 2 - 层级遍历与按名/按类型查找 (Priority: P2)

Traverse、FindByName、FindByType、GetPath、GetId、SetActive、GetActive；HierarchyIterator 单次有效。

**Why this priority**: 下游依赖层级查询与激活状态。

**Independent Test**: 在已有场景图上 Traverse/FindByName/FindByType，GetPath/GetId，SetActive/GetActive；验证迭代器单次有效。

**Acceptance Scenarios**:

1. **Given** WorldRef 与 root NodeId，**When** Traverse(world, root)，**Then** 返回单次有效迭代器，可 Next/GetId 直至 IsValid 为 false。
2. **Given** 子树与名称，**When** FindByName(world, root, name)，**Then** 返回指向首个匹配或 end 的单次有效迭代器。
3. **Given** 节点，**When** SetActive(node, false) 再 GetActive(node)，**Then** 返回 false。

---

### User Story 3 - 多 World 与当前活动场景 (Priority: P2)

AddWorld、SetActiveWorld、GetCurrentWorld；SetActiveWorld 下一帧或下一次 UpdateTransforms 生效。

**Independent Test**: AddWorld、SetActiveWorld 后本帧 GetCurrentWorld 仍为旧 WorldRef，下一帧或下一次 UpdateTransforms 后为新 WorldRef。

**Acceptance Scenarios**:

1. **Given** 无额外 World，**When** AddWorld()，**Then** 返回新 WorldRef。
2. **Given** 已调用 SetActiveWorld(newWorld)，**When** 本帧内 GetCurrentWorld()，**Then** 仍返回旧 WorldRef；当 UpdateTransforms 或下一帧后，GetCurrentWorld() 返回 newWorld。

---

### User Story 4 - Level 加载/卸载与 Resource 对接 (Priority: P3)

LoadLevel、UnloadLevel、Level 资源句柄；与 013-Resource 约定句柄与加载时机。

**Independent Test**: 在与 013-Resource 约定后，LoadLevel/UnloadLevel 验证关卡边界与句柄生命周期。

**Acceptance Scenarios**:

1. **Given** WorldRef 与资源路径或句柄，**When** LoadLevel(world, ...)，**Then** 返回 LevelHandle（或占位实现）；UnloadLevel(world, handle) 可卸载。

---

### User Story 5 - Entity 根实体挂接 (Priority: P3)

在当前（或指定）World 上挂接 Entity 根实体；接口归属与签名由 plan 与 005-Entity 约定。

**Independent Test**: 在 plan 与 005-Entity 约定接口后，挂接根实体，下游可基于 WorldRef/NodeId 与 Entity 协作。

**Acceptance Scenarios**:

1. **Given** 与 005-Entity 约定后的 API，**When** 挂接根实体，**Then** 下游可查询/操作与场景节点关联的实体。

---

### Edge Cases

- SetParent 导致成环或自引用时返回 false，场景图不变。
- Traverse/Find 返回的 HierarchyIterator 遍历结束后不可复用；移动后原迭代器失效。
- UpdateTransforms 未调用前 GetWorldTransform 行为由实现约定（如上一帧缓存）。
- LoadLevel/UnloadLevel 在与 013-Resource 对接前可为占位或最小签名。

## Requirements *(mandatory)*

### Functional Requirements

- **FR-001**: 系统 MUST 在指定 World 下提供 CreateNode，返回 NodeId。
- **FR-002**: 系统 MUST 提供 SetParent(node, parent)；成环或非法层级时返回 false 且不修改场景图。
- **FR-003**: 系统 MUST 提供 GetParent、GetChildren、GetLocalTransform、SetLocalTransform、GetWorldTransform、SetDirty、UpdateTransforms，语义与契约一致。
- **FR-004**: 系统 MUST 提供 Traverse、FindByName、FindByType、GetPath、GetId、SetActive、GetActive；HierarchyIterator 单次有效、可移动。
- **FR-005**: 系统 MUST 提供 GetCurrentWorld、SetActiveWorld、AddWorld；SetActiveWorld 在下一帧或下次 UpdateTransforms 生效。
- **FR-006**: 系统 MUST 提供 LoadLevel、UnloadLevel、LevelHandle，与 013-Resource 约定一致（或占位）。
- **FR-007**: 对外仅暴露契约（`specs/_contracts/004-scene-public-api.md`）与 plan 已声明的类型与 API；不新增契约外接口。

### Key Entities

- **WorldRef**：场景/世界容器引用；生命周期为加载后直至卸载。
- **NodeId**：场景图节点 ID；父子关系、层级路径。
- **Transform**：局部/世界变换（与 Core.Math 或共用类型一致）。
- **HierarchyIterator**：层级遍历与查找用迭代器；单次有效。
- **LevelHandle**：关卡加载/卸载边界；与 Resource 契约一致。

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: 场景图 API（CreateNode、SetParent、GetParent、GetChildren、Local/WorldTransform、SetDirty、UpdateTransforms）可独立测试并通过。
- **SC-002**: 层级 API（Traverse、FindByName、FindByType、GetPath、GetId、SetActive、GetActive）可独立测试；迭代器单次有效可验证。
- **SC-003**: 多 World API（AddWorld、SetActiveWorld、GetCurrentWorld）可独立测试；生效时机符合契约。
- **SC-004**: 对外头文件与符号仅包含契约与 ABI 已声明内容；无契约外泄漏。

## Interface Contracts *(multi-agent sync)*

- **本模块实现的契约**：`specs/_contracts/004-scene-public-api.md`
- **本模块依赖的契约**：见下方 Dependencies
- **ABI/构建**：须实现 `specs/_contracts/004-scene-ABI.md` 中全部符号；构建须引入真实子模块代码；接口变更须在 ABI 中更新。

## Dependencies

- **001-Core**：`specs/_contracts/001-core-public-api.md`（数学、容器、内存、日志）；实现时只使用该契约已声明的类型与 API。
- **002-Object**：`specs/_contracts/002-object-public-api.md`（序列化、类型信息）；实现时只使用该契约已声明的类型与 API。
- **013-Resource**（Level 加载）：与 013-Resource 契约约定后使用；未约定前 LoadLevel/UnloadLevel 可为占位。
- **005-Entity**（根实体挂接）：与 005-Entity 契约约定后补全接口；未约定前可为占位。
- 依赖关系总览：`specs/_contracts/000-module-dependency-map.md`。
