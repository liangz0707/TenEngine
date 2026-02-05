# Implementation Plan: 004-Scene 完整功能集

**Branch**: 004-scene-fullversion-001 | **Date**: 2026-01-29 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/004-scene-fullversion-001/spec.md`

**规约与契约**：`docs/module-specs/004-scene.md`、`specs/_contracts/004-scene-public-api.md`。本计划仅暴露契约已声明的类型与 API，技术栈 C++17、CMake，本切片实现完整功能集（SceneGraph、Hierarchy、World、Level、激活/禁用、Entity 根挂接）。

## Summary

实现 004-Scene 模块完整功能集：场景图（Node、Parent/Children、LocalTransform、WorldTransform、SetDirty、UpdateTransforms）、层级（Traverse、FindByName/ByType、GetPath、GetId、SetActive）、多 World（GetCurrentWorld、SetActiveWorld、AddWorld）、Level（LoadLevel、UnloadLevel、Level 资源句柄）、激活/禁用、与 Entity 根挂接。对外 API 严格以契约为准；Transform 与 Core.Math 或共用类型一致；HierarchyIterator 单次有效；SetActiveWorld 下一帧或下一次 UpdateTransforms 生效；父子成环/非法层级拒绝并返回错误；LoadLevel/UnloadLevel 与 Entity 根挂接的细节由 plan 与 013-Resource、005-Entity 对接约定。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake 3.16+，单一日志构建  
**Primary Dependencies**: 001-Core（数学、容器、内存、日志）、002-Object（序列化、类型信息）；013-Resource、005-Entity 接口在对接时约定  
**Storage**: 内存；场景图节点、变换缓存、层级索引；无直接文件/GPU，关卡数据通过 Resource 加载  
**Testing**: 单元测试（场景图、变换、层级、多 World、成环拒绝）；集成测试（Traverse/Find 单次有效、SetActiveWorld 生效时机、与 Resource/Entity 对接）  
**Target Platform**: 与 Core 一致（Windows/Linux/macOS 等）  
**Project Type**: 引擎静态库或动态库，供 Entity、Pipeline、Editor 等链接  
**Performance Goals**: 变换更新与脏标记一致；Traverse/Find 单次遍历可接受线性开销；具体帧预算在实现文档中约定  
**Constraints**: 仅暴露契约已声明的类型与 API；不实现规约与契约未列出的能力  
**Scale/Scope**: 本切片为完整功能集，覆盖规约中 SceneGraph、Hierarchy、World、Level、激活/禁用、Entity 根挂接

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 |
|------|------|
| I. Modular Renderer Architecture | 通过：004-Scene 为有界模块，职责明确，可独立测试。 |
| II. Modern Graphics API First | 不适用：本模块无直接 GPU API。 |
| III. Data-Driven Pipeline | 通过：场景/关卡数据通过 Resource 加载，本模块消费描述与引用。 |
| IV. Performance & Observability | 通过：变换更新与脏标记可测；日志与契约行为可验证。 |
| V. Versioning & ABI Stability | 通过：公开 API 以契约为准，破坏性变更递增 MAJOR。 |
| VI. Module Boundaries & Contract-First | 通过：仅暴露契约已声明的类型与 API；实现仅使用 001/002 契约及与 013/005 约定的接口。 |
| Technology Stack | 通过：C++17、CMake。 |
| Code Quality & Testing | 通过：单元/集成测试覆盖契约保证；公开 API 有测试。 |

无违规；无需填写 Complexity Tracking。

## Project Structure

### Documentation (this feature)

```text
specs/004-scene-fullversion-001/
├── plan.md              # 本文件
├── research.md          # Phase 0 产出
├── data-model.md        # Phase 1 产出
├── quickstart.md        # Phase 1 产出
├── checklists/
│   └── requirements.md
└── tasks.md             # Phase 2 产出（/speckit.tasks）
```

### Source Code (repository root)

```text
# 以单库形式交付 004-Scene；路径以仓库实际布局为准
src/
├── scene/               # 004-Scene 实现
│   ├── scene_graph.hpp   # Node、Parent/Children、Local/WorldTransform、SetDirty、UpdateTransforms
│   ├── hierarchy.hpp     # Traverse、FindByName/ByType、GetPath、GetId、SetActive
│   ├── world.hpp         # GetCurrentWorld、SetActiveWorld、AddWorld
│   ├── level.hpp         # LoadLevel、UnloadLevel、Level 句柄（与 013 约定）
│   └── (entity_root 与 005 约定后可选)
├── ...
tests/
├── scene/
│   ├── unit/             # 场景图、变换、层级、多 World、成环拒绝
│   └── integration/      # Traverse 单次有效、SetActiveWorld 时机、Resource/Entity 对接
└── ...
```

**Structure Decision**: 单库、单 CMake 子工程（或纳入现有 engine 的 scene 子目录）；对外头文件仅暴露契约中的类型与函数声明，实现细节在 .cpp 或内部头文件中。

## Phase 0: Research

- 已完成：见 [research.md](research.md)。技术栈 C++17/CMake、依赖边界、HierarchyIterator 单次有效、SetActiveWorld 生效时机、成环拒绝、LoadLevel/Entity 根挂接由 plan 与 013/005 约定均已确认。

## Phase 1: Design & Contracts

- **Data model**: 见 [data-model.md](data-model.md)。  
- **Quickstart**: 见 [quickstart.md](quickstart.md)。  
- **API 雏形（契约更新）**：见本文件末尾「契约更新（API 雏形）」一节；可直接粘贴到 `specs/_contracts/004-scene-public-api.md` 的「API 雏形」小节。

## Complexity Tracking

无违规；本表留空。

---

## 契约更新（API 雏形）

以下内容可直接粘贴到 `specs/_contracts/004-scene-public-api.md` 的「API 雏形」小节。仅暴露契约已声明的类型与能力；Transform 与 Core.Math 或共用类型一致；具体命名空间与头文件路径由实现约定。

```markdown
## API 雏形（简化声明）

### 类型与句柄（C++17）

- **WorldRef**：不透明句柄，表示场景/世界容器；生命周期为加载后直至卸载。  
  - 实现可采用 `using WorldRef = void*;` 或 `struct WorldRef { void* handle; };` 等，由 ABI 约定。
- **NodeId**：不透明句柄，表示场景图节点 ID；与节点同生命周期。  
  - 实现可采用 `using NodeId = void*;` 或 `struct NodeId { uintptr_t id; };` 等。
- **Transform**：局部/世界变换，与 **Core.Math** 或共用数学类型一致（如 `struct Transform { Vector3 position; Quaternion rotation; Vector3 scale; };` 或 4×4 矩阵）；与节点/实体同步。
- **HierarchyIterator**：层级遍历、按名/按类型查找用迭代器；**单次有效**（遍历结束后即失效，不可复用）。  
  - 实现为可移动类型，Traverse/FindByName/FindByType 返回；调用方不得在遍历结束后再使用。
- **Active**：节点/子树是否参与更新/渲染；与节点绑定。  
  - 使用 `bool` 或等价类型；SetActive 设置，下游可查询。
- **LevelHandle**：关卡加载/卸载边界；与 **013-Resource** 约定。  
  - 不透明句柄；LoadLevel 返回，UnloadLevel 参数；生命周期按 Resource 契约。

（以上类型名与 Core.Math 的 Vector3/Quaternion 等以 001-core-public-api 及项目共用头文件为准。）

### 场景图（SceneGraph）

- `NodeId CreateNode(WorldRef world);`  
  - 在指定 World 下创建节点，返回 NodeId；失败返回无效 NodeId（由实现约定语义）。
- `bool SetParent(NodeId node, NodeId parent);`  
  - 将 `node` 的父节点设为 `parent`；若会导致父子成环或非法层级，返回 `false` 且不修改场景图；成功返回 `true`。
- `NodeId GetParent(NodeId node);`  
  - 返回节点的父节点 NodeId；无父节点时返回无效 NodeId。
- `void GetChildren(NodeId node, /* out */ NodeId* children, size_t* count);`  
  - 或使用契约允许的容器类型返回子节点列表；具体签名与 001-Core 容器一致。
- `Transform GetLocalTransform(NodeId node);`  
- `void SetLocalTransform(NodeId node, Transform const& t);`  
- `Transform GetWorldTransform(NodeId node);`  
  - WorldTransform 由 UpdateTransforms 更新；未更新前行为由实现约定（如上一帧缓存）。
- `void SetDirty(NodeId node);`  
  - 标记节点（及子树）为脏，下次 UpdateTransforms 时更新 WorldTransform。
- `void UpdateTransforms(WorldRef world);`  
  - 按依赖顺序更新该 World 下所有脏节点的 WorldTransform，并清除脏标记。  
  - 多 World 时，仅更新传入的 World；当前活动 World 的切换在下一帧或本次 UpdateTransforms 入口生效（见 SetActiveWorld）。

### 层级（Hierarchy）

- `HierarchyIterator Traverse(WorldRef world, NodeId root);`  
  - 从 `root` 起按层级顺序遍历子树；返回**单次有效**迭代器，遍历结束后即失效。
- `HierarchyIterator FindByName(WorldRef world, NodeId root, char const* name);`  
  - 在 `root` 子树内按名称查找；返回单次有效迭代器（指向首个匹配或 end）。
- `HierarchyIterator FindByType(WorldRef world, NodeId root, /* TypeId 或等价 */ void* typeFilter);`  
  - 在 `root` 子树内按类型查找；类型过滤与 002-Object 约定一致；返回单次有效迭代器。
- `void GetPath(NodeId node, /* out */ char* pathBuffer, size_t bufferSize);`  
  - 或返回契约允许的字符串类型；路径格式由实现约定。
- `NodeId GetId(HierarchyIterator const& it);`  
  - 返回迭代器当前指向的 NodeId；迭代器失效后调用未定义。
- `void SetActive(NodeId node, bool active);`  
  - 设置节点（及可选子树）的 Active 状态；下游可查询以决定是否参与更新/渲染。
- `bool GetActive(NodeId node);`  
  - 查询节点 Active 状态。

（HierarchyIterator 的 `Next`/`IsValid` 等由实现提供，契约保证单次有效、不可复用。）

### World / Level

- `WorldRef GetCurrentWorld();`  
  - 返回当前活动 WorldRef；多 World 时，SetActiveWorld 在下一帧或下一次 UpdateTransforms 后生效。
- `void SetActiveWorld(WorldRef world);`  
  - 将当前活动 World 设为 `world`；**下一帧或下一次 UpdateTransforms 后**生效，本帧内 GetCurrentWorld 仍返回旧 WorldRef。
- `WorldRef AddWorld();`  
  - 创建并返回新 WorldRef；生命周期为加载后直至显式卸载。
- `/* 与 013-Resource 约定 */ LevelHandle LoadLevel(WorldRef world, /* Resource 句柄或路径等 */ ...);`  
  - 在指定 World 加载关卡；句柄、参数与失败语义由 plan 与 013-Resource 对接约定。
- `void UnloadLevel(WorldRef world, LevelHandle level);`  
  - 卸载指定 Level；语义与 Resource 契约一致，由对接约定。

### Entity 根挂接

- 与 **005-Entity** 对接约定：挂接根实体的接口归属（Scene 提供 AttachEntityRoot 或 Entity 提供 AttachToWorld）及签名在 plan 与 005-Entity 对接后补全；本雏形仅预留“在当前（或指定）World 上挂接 Entity 根实体”的语义，具体 API 写入契约时与 005-Entity 同步。
```

**说明**：  
- 上述声明为简化雏形，实现时命名空间、头文件路径、错误码（若用 Result 替代 bool）等以仓库与 T0-contracts 约定为准。  
- Transform、容器类型（GetChildren 返回）、字符串类型（GetPath）须与 001-Core、002-Object 契约一致。  
- LoadLevel/UnloadLevel、Entity 根挂接的最终签名在与 013-Resource、005-Entity 对接后写回契约。
