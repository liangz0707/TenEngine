# Implementation Plan: 004-Scene Full Module

**Branch**: `004-scene-fullmodule-001` | **Date**: 2026-02-04 | **Spec**: [spec.md](spec.md)  
**Input**: Feature specification from `specs/004-scene-fullmodule-001/spec.md`

**规约与契约**：`docs/module-specs/004-scene.md`、`specs/_contracts/004-scene-public-api.md`。上游契约：`001-core-public-api.md`、`002-object-public-api.md`。设计参考 Unity SceneManager/Transform、UE UWorld/Level。

## Summary

实现 004-Scene 完整模块：场景图（CreateNode、SetParent、GetParent/GetChildren、Local/WorldTransform、SetDirty、UpdateTransforms）、层级（Traverse、FindByName/FindByType、GetPath、GetId、SetActive/GetActive）、多 World（GetCurrentWorld、SetActiveWorld、AddWorld）、Level（LoadLevel、UnloadLevel、LevelHandle）、以及 ABI 中 **TODO 必须项**（LevelAssetDesc/SceneNodeDesc/TransformDesc 注册与反序列化、GetNodeModelGuid、GetNodeEntityPrefabGuid）。技术栈 C++17、CMake；对外仅暴露契约与 ABI 已声明类型与接口。全量 ABI 见 `specs/004-scene-fullmodule-001/contracts/004-scene-ABI-full.md`；tasks 与 implement 基于该全量内容实现。

## 实现范围（TenEngine：实现全量 ABI 内容）

- **全量 ABI 文档**：`specs/004-scene-fullmodule-001/contracts/004-scene-ABI-full.md`（原始 ABI + 新增 + 修改）。
- **原始 ABI**：`specs/_contracts/004-scene-ABI.md` 中已声明的类型与枚举、场景加载、ISceneWorld、ISceneNode 全部条目。
- **新增 ABI**：数据与注册（TODO 必须实现）— 见下方「契约更新」小节。
- **修改 ABI**：无（本 feature 不修改现有 ABI 行）。
- **实现约束**：tasks 和 implement **必须基于全量 ABI 内容**（含契约 API、SceneManager 风格、ISceneWorld/ISceneNode、数据与注册）进行实现；不得仅实现变化部分。ABI 中的 TODO 为本 feature **必须增加**的部分。

## Technical Context

**Language/Version**: C++17  
**Build**: CMake 3.16+，单一日志构建  
**Primary Dependencies**: 001-Core（数学、容器、内存、日志）、002-Object（序列化、类型信息）；013-Resource、005-Entity 接口在对接时约定  
**Storage**: 内存；场景图节点、变换缓存、层级索引；关卡数据通过 Resource 加载  
**Testing**: 单元测试（场景图、变换、层级、多 World、成环拒绝）；集成测试（Traverse/Find 单次有效、SetActiveWorld 时机）  
**Target Platform**: 与 Core 一致（Windows/Linux/macOS 等）  
**Project Type**: 引擎静态库，供 Entity、Pipeline、Editor 等链接  
**Constraints**: 仅暴露契约与 ABI 已声明的类型与 API；变换更新与脏标记一致；SetActiveWorld 下一帧或下次 UpdateTransforms 生效  
**Scale/Scope**: 本 feature 为完整模块，覆盖规约 SceneGraph、Hierarchy、World、Level、激活/禁用及 ABI TODO。

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | TenEngineHelpers / tenengine_resolve_my_dependencies 引入上游源码构建。 |
| 002-object | **源码** | 同上。 |

**说明**：当前仅支持源码引入；013-Resource、005-Entity 未对接前可不引入或占位。

### 第三方依赖（本 feature 涉及模块所需）

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| 本 feature 无第三方依赖 | — | — | 数学与容器以 001-Core 为准。 |

## Constitution Check

| 原则 | 状态 |
|------|------|
| I. Modular Renderer Architecture | 通过：004-Scene 有界模块，可独立测试。 |
| II. Modern Graphics API First | 不适用：本模块无直接 GPU API。 |
| III. Data-Driven Pipeline | 通过：关卡数据通过 Resource 加载。 |
| IV. Performance & Observability | 通过：变换更新与脏标记可测。 |
| V. Versioning & ABI Stability | 通过：公开 API 以契约与 ABI 为准。 |
| VI. Module Boundaries & Contract-First | 通过：仅暴露契约与 ABI 已声明接口；实现全量 ABI；依赖以源码引入。 |
| Technology Stack | 通过：C++17、CMake。 |
| Code Quality & Testing | 通过：单元/集成测试覆盖契约与 ABI 保证。 |

无违规；无需填写 Complexity Tracking。

## Project Structure

### Documentation (this feature)

```text
specs/004-scene-fullmodule-001/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── 004-scene-ABI-full.md   # 全量 ABI（实现参考）
├── checklists/
│   └── requirements.md
└── tasks.md                    # /speckit.tasks 产出
```

### Source Code (repository root)

```text
src/
├── scene/
│   ├── types.hpp
│   ├── scene_graph.hpp/.cpp
│   ├── hierarchy.hpp/.cpp
│   ├── world.hpp/.cpp
│   ├── level.hpp/.cpp
│   └── detail/                 # 内部用，不暴露契约外
tests/
├── scene/
│   ├── unit/
│   └── integration/
```

**Structure Decision**: 单库、单 CMake 子工程；对外头文件仅暴露契约与 ABI 中的类型与函数。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> **说明**：以下为相对于现有 `specs/_contracts/004-scene-ABI.md` 的**新增**条目（对应 ABI 中 TODO 必须实现部分）。写回时将此表**增补**到 `specs/_contracts/004-scene-ABI.md`。
>
> **实现时使用全量内容**：tasks 和 implement 须基于 `specs/004-scene-fullmodule-001/contracts/004-scene-ABI-full.md` 全量 ABI 实现。

| 操作 | 模块名 | 命名空间 | 类名 | 接口说明 | 头文件 | 符号 | 说明 |
|------|--------|----------|------|----------|--------|------|------|
| 新增 | 004-Scene | te::scene | — | 引擎启动时完成 LevelAssetDesc、SceneNodeDesc 类型注册 | 004 初始化 | RegisterType&lt;LevelAssetDesc&gt;, RegisterType&lt;SceneNodeDesc&gt; | 通过 002 注册 |
| 新增 | 004-Scene | te::scene | LevelAssetDesc | 关卡资源描述（反序列化） | te/scene/LevelTypes.h | LevelAssetDesc | formatVersion, debugDescription, rootNodes, defaultWorldSettings |
| 新增 | 004-Scene | te::scene | SceneNodeDesc | 场景节点描述（反序列化） | te/scene/SceneNodeTypes.h | SceneNodeDesc | name, localTransform, children, modelGuid, entityPrefabGuid, components, active |
| 新增 | 004-Scene | te::scene | TransformDesc | 变换描述 | te/scene/SceneTypes.h | TransformDesc | position, rotation, scale |
| 新增 | 004-Scene | te::scene | — | 节点模型 GUID | te/scene/SceneNode.h | GetNodeModelGuid | `ResourceId GetNodeModelGuid(ISceneNode* node);` 与 013 约定 |
| 新增 | 004-Scene | te::scene | — | 节点实体预制 GUID | te/scene/SceneNode.h | GetNodeEntityPrefabGuid | `ResourceId GetNodeEntityPrefabGuid(ISceneNode* node);` 与 005/013 约定 |

**反序列化与流程**（写回时可将以下说明加入 ABI 的「数据相关 TODO」或新小节）：  
- LoadScene(pathOrResourceId, mode) → 001.FileRead 读 .level → 002.Deserialize 得到 LevelAssetDesc → 构建 ISceneWorld、ISceneNode 树；节点存储 modelGuid、entityPrefabGuid。  
- UnloadScene(scene)：销毁场景图与节点数据；与 013-Resource 协同。

## Complexity Tracking

无违规；本表留空。
