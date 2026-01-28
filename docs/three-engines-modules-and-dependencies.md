# 三引擎模块详细对照与依赖图

本文档提供 **TenEngine**、**Unreal Engine**、**Unity** 的模块划分详细对照及依赖图，便于架构对齐与设计参考。依赖关系以官方文档与公开资料为准，具体实现以各引擎源码/包清单为准。

**T0 当前契约**：27 模块，契约见 `specs/_contracts/000-module-dependency-map.md`（`001-core-public-api` … `027-xr-public-api`、边界 `pipeline-to-rci`）。下表为早期 001–006 规格，保留作对照。

---

## 一、TenEngine 模块与依赖

### 1.1 模块列表（详细，早期 001–006）

| 编号 | 模块名 (spec) | 职责摘要 | 对外契约 |
|------|----------------|----------|----------|
| 001 | **engine-core-module** | 内存管理、线程管理、ECS、平台抽象、序列化；动态库交付 | core-public-api.md |
| 002 | **rendering-rci-interface** | 图形 API 抽象（RCI）；统一渲染接口，多后端（DLL/源码） | rci-public-api.md |
| 003 | **editor-system** | 面板、布局、文件管理、渲染窗口、属性面板、场景树 | 依赖 RCI、Core |
| 004 | **resource-system** | 资源导入、同步/异步加载卸载、加载状态与限制 | 依赖 Core |
| 005 | **shader-system** | Shader 读取、编译、预编译、变体；库/源码/工具形态 | 依赖 Core、RCI |
| 006 | **render-pipeline-system** | 场景收集→DrawCall→抽象命令缓冲→RCI | pipeline-to-rci.md |
| 006 | **thirdparty-integration-tool** | 第三方集成工具 | 按实现 |

### 1.2 TenEngine 依赖表（下游 → 上游）

| 模块 | 直接依赖 | 契约文件 |
|------|----------|----------|
| 001-engine-core-module | — | 无（根） |
| 002-rendering-rci-interface | 001 | core-public-api.md |
| 003-editor-system | 001, 002 | core-public-api.md, rci-public-api.md |
| 004-resource-system | 001 | core-public-api.md |
| 005-shader-system | 001, 002 | core-public-api.md, rci-public-api.md |
| 006-render-pipeline-system | 002 | rci-public-api.md, pipeline-to-rci.md |
| 006-thirdparty-integration-tool | 按实现 | — |

### 1.3 TenEngine 依赖图（ASCII）

```
                         ┌─────────────────────────┐
                         │ 003-editor-system       │
                         │ (Editor, 渲染窗口/场景树) │
                         └───────────┬─────────────┘
                                     │
    ┌────────────────────┐           │           ┌────────────────────┐
    │ 004-resource-system│           │           │ 005-shader-system  │
    │ (资源导入/加载/卸载) │           │           │ (Shader 编译/变体)  │
    └─────────┬──────────┘           │           └─────────┬──────────┘
              │                      │                     │
              │                      ▼                     │
              │             ┌───────────────────┐          │
              │             │ 006-render-       │          │
              │             │ pipeline-system   │◄─────────┘
              │             │ (场景→DrawCall→   │
              │             │  CommandBuffer)   │
              │             └─────────┬─────────┘
              │                       │
              │                       ▼
              │             ┌───────────────────┐
              └────────────►│ 002-rendering-rci │
                            │ (RCI 图形抽象层)   │
                            └─────────┬─────────┘
                                      │
                                      ▼
                            ┌───────────────────┐
                            │ 001-engine-core   │
                            │ (内存/线程/ECS/   │
                            │  平台/序列化)      │
                            └───────────────────┘
```

---

## 二、Unreal Engine 模块与依赖

### 2.1 模块列表（详细，按层级）

**底层（无引擎业务依赖）**

| 模块 | 职责 | 典型依赖 |
|------|------|----------|
| **Core** | 基础类型、容器、字符串、平台抽象、模块管理（FModuleManager）、线程、内存 | — |
| **CoreUObject** | UObject、反射、序列化、属性系统 | Core |
| **ApplicationCore** | 应用生命周期、窗口/消息 | Core |

**运行时核心**

| 模块 | 职责 | 典型依赖 |
|------|------|----------|
| **Engine** | 世界、Actor、组件、关卡、游戏框架 | Core, CoreUObject, ApplicationCore |
| **RHI** (Render Hardware Interface) | 图形 API 抽象（D3D12/Vulkan/Metal）；命令列表、资源、PSO | Core |
| **RHICore** | RHI 扩展、通用渲染工具 | Core, RHI |
| **RenderCore** | 虚拟纹理、Shader 参数、渲染资源描述 | Core, RHI, RHICore |
| **Renderer** | 高层渲染、Mesh Draw Command、Pass、RDG 使用 | Engine, RenderCore, RHI |
| **SlateCore** | UI 核心（布局、绘制、输入） | Core, ApplicationCore |
| **Slate** | 高层 UI 控件 | SlateCore, InputCore |
| **InputCore** | 输入抽象 | Core, ApplicationCore |

**编辑器（仅 Editor）**

| 模块 | 职责 | 典型依赖 |
|------|------|----------|
| **UnrealEd** | 编辑器核心、资源编辑 | Engine, Slate, SlateCore |
| **EditorStyle** | 编辑器样式 | Slate, SlateCore |
| **LevelEditor** | 关卡编辑器 | UnrealEd, Slate |

**渲染管线（RDG）**

| 概念 | 说明 |
|------|------|
| **RDG** | Render Dependency Graph；Setup 阶段建图，Execute 阶段执行 Pass；Pass 参数声明资源读写，推导依赖与生命周期。 |
| **依赖链** | RHI → RHICore → RenderCore → Renderer；Renderer 内使用 RDG 组织 Pass。 |

### 2.2 Unreal 依赖表（节选：核心与渲染）

| 模块 | 直接依赖（典型） |
|------|------------------|
| Core | — |
| CoreUObject | Core |
| ApplicationCore | Core |
| Engine | Core, CoreUObject, ApplicationCore, … |
| RHI | Core |
| RHICore | Core, RHI |
| RenderCore | Core, RHI, RHICore |
| Renderer | Engine, RenderCore, RHI, … |
| SlateCore | Core, ApplicationCore |
| Slate | SlateCore, InputCore |
| UnrealEd | Engine, Slate, SlateCore, … |

### 2.3 Unreal 依赖图（ASCII，核心+渲染+编辑器）

```
                    ┌─────────────────────────────────┐
                    │ UnrealEd / LevelEditor (Editor)  │
                    └─────────────────┬───────────────┘
                                      │
    ┌─────────────┐                   │                   ┌─────────────┐
    │   Slate     │                   │                   │  Renderer   │
    │   (UI)      │                   │                   │ (RDG, Pass) │
    └──────┬──────┘                   │                   └──────┬──────┘
           │                         │                          │
           ▼                         ▼                          ▼
    ┌─────────────┐           ┌─────────────┐           ┌─────────────┐
    │ SlateCore   │           │   Engine    │           │ RenderCore  │
    │ InputCore   │           │ (World/Actor)│           │ (Shader,VT) │
    └──────┬──────┘           └──────┬──────┘           └──────┬──────┘
           │                         │                          │
           │                         │                          ▼
           │                         │                   ┌─────────────┐
           │                         │                   │  RHICore    │
           │                         │                   └──────┬──────┘
           │                         │                          │
           ▼                         ▼                          ▼
    ┌─────────────────────────────────────────────────────────────────┐
    │ ApplicationCore                                                 │
    └─────────────────────────────────────────────────────────────────┘
                                      │
           ┌──────────────────────────┼──────────────────────────┐
           ▼                          ▼                          ▼
    ┌─────────────┐           ┌─────────────┐             ┌─────────────┐
    │ CoreUObject │           │    Core     │             │     RHI     │
    │ (UObject)   │           │ (基础/平台)  │             │ (图形API)   │
    └──────┬──────┘           └──────┬──────┘             └──────┬──────┘
           │                         │                          │
           └─────────────────────────┴──────────────────────────┘
                                     │
                                     ▼
                              ┌─────────────┐
                              │    Core     │
                              │ (根模块)    │
                              └─────────────┘
```

---

## 三、Unity 模块与依赖

### 3.1 包/模块列表（详细，按层级）

**核心引擎（内置，C++ 层 + C# 封装）**

| 包/模块 | 职责 | 依赖 |
|---------|------|------|
| **Unity Engine Core** | 运行时核心、场景、GameObject、组件、脚本后端 | — |
| **com.unity.modules.subsystems** | 子系统定义与运行时（Descriptor、Store、Start/Stop） | Core |
| **com.unity.modules.animation** | 动画 | Core |
| **com.unity.modules.assetbundle** | Asset Bundle | Core |
| **com.unity.modules.audio** | 音频 | Core |
| **com.unity.modules.imageconversion** | 图像转换 | Core |
| **com.unity.modules.imgui** | IMGUI | Core |
| **com.unity.modules.physics** | 物理 | Core |
| **com.unity.modules.physics2d** | 2D 物理 | Core |
| **com.unity.modules.terrain** | 地形 | Core |
| **com.unity.modules.ui** | UI 基础 | Core |
| **com.unity.modules.xr** | XR 基础 | Core |

**渲染管线（SRP）**

| 包 | 职责 | 依赖 |
|----|------|------|
| **com.unity.render-pipelines.core** | SRP 核心、Shader 库、通用渲染工具 | Engine Core |
| **com.unity.render-pipelines.universal** (URP) | 通用渲染管线 | render-pipelines.core |
| **com.unity.render-pipelines.high-definition** (HDRP) | 高清渲染管线 | render-pipelines.core |
| **com.unity.shadergraph** | Shader 可视化 | render-pipelines.core |
| **com.unity.visualeffectgraph** | VFX Graph | render-pipelines.core |
| **com.unity.rendering.denoising** | 降噪 | 渲染相关 |

**资源与内容**

| 包 | 职责 | 依赖 |
|----|------|------|
| **com.unity.addressables** | 可寻址资源、异步加载 | Core |
| **com.unity.scriptablebuildpipeline** | 可编程构建管线 | Core |

**DOTS / 实体**

| 包 | 职责 | 依赖 |
|----|------|------|
| **com.unity.collections** | 高性能容器 | Core |
| **com.unity.entities** | ECS 实体组件系统 | Core, Collections |
| **com.unity.physics** | Unity Physics（DOTS） | Entities |
| **com.unity.entities.graphics** | 实体与渲染桥接 | Entities, 渲染管线 |

**编辑器与工具**

| 包 | 职责 | 依赖 |
|----|------|------|
| **com.unity.editor** | 编辑器核心 | Engine |
| **com.unity.ugui** | Unity UI (uGUI) | Core |
| **com.unity.inputsystem** | 新输入系统 | Core, Subsystems |

### 3.2 Unity 依赖表（节选：核心与渲染）

| 包 | 直接依赖（典型） |
|----|------------------|
| Engine Core | — |
| com.unity.modules.subsystems | Core |
| com.unity.render-pipelines.core | Core |
| com.unity.render-pipelines.universal | render-pipelines.core |
| com.unity.render-pipelines.high-definition | render-pipelines.core |
| com.unity.entities | Core, com.unity.collections |
| com.unity.entities.graphics | entities, 渲染管线 |
| com.unity.addressables | Core |

### 3.3 Unity 依赖图（ASCII，核心+渲染+实体）

```
    ┌─────────────────────────────────────────────────────────────────┐
    │ Editor / uGUI / Input System / Addressables / XR / 其他功能包     │
    └─────────────────────────────┬───────────────────────────────────┘
                                  │
    ┌─────────────────────────────┼─────────────────────────────┐
    │                             │                             │
    ▼                             ▼                             ▼
┌───────────────┐         ┌───────────────┐         ┌───────────────┐
│ URP / HDRP    │         │ Entities.     │         │ Subsystems     │
│ (二选一)      │         │ Graphics      │         │ (Input/XR…)   │
└───────┬───────┘         └───────┬───────┘         └───────┬───────┘
        │                         │                         │
        ▼                         ▼                         │
┌───────────────┐         ┌───────────────┐                 │
│ SRP Core      │         │ Entities      │                 │
│ (render-      │         │ (ECS)         │                 │
│ pipelines.    │         └───────┬───────┘                 │
│ core)         │                 │                         │
└───────┬───────┘                 ▼                         │
        │                 ┌───────────────┐                 │
        │                 │ Collections   │                 │
        │                 └───────┬───────┘                 │
        │                         │                         │
        └─────────────────────────┼─────────────────────────┘
                                  │
                                  ▼
                        ┌───────────────────┐
                        │ Unity Engine Core │
                        │ (Native + C# API) │
                        └───────────────────┘
```

---

## 四、三引擎模块对照表

### 4.1 按功能域对照

| 功能域 | TenEngine | Unreal Engine | Unity |
|--------|-----------|---------------|-------|
| **核心/基础** | 001-engine-core（内存/线程/ECS/平台/序列化） | Core, CoreUObject, ApplicationCore | Engine Core, com.unity.modules.* |
| **对象/实体** | Core 内 ECS | Engine (Actor/Component), CoreUObject | GameObject/Component, com.unity.entities |
| **图形 API 抽象** | 002-rendering-rci (RCI) | RHI, RHICore | 由 SRP Core 与底层封装 |
| **渲染管线/高层渲染** | 006-render-pipeline + 002 RCI | RenderCore, Renderer, RDG | render-pipelines.core, URP/HDRP |
| **Shader** | 005-shader-system | RenderCore/Renderer 内 + 材质系统 | Shader Graph, 内置 Shader 库 |
| **资源** | 004-resource-system | Engine 内资源系统 + 插件 | Addressables, Asset Bundle, 内置导入 |
| **编辑器/工具** | 003-editor-system | UnrealEd, Slate, LevelEditor | com.unity.editor, 各工具包 |
| **UI** | 003 内面板/布局 | Slate, SlateCore, InputCore | uGUI, Input System, UI Toolkit |
| **可插拔/扩展** | 契约 + 动态库 | 模块 + 插件 (.uplugin) | Package + Subsystems |

### 4.2 按依赖层级对照（概念层）

| 层级 | TenEngine | Unreal | Unity |
|------|-----------|--------|-------|
| **L0 根** | 001-engine-core | Core | Engine Core |
| **L1 对象/平台** | — | CoreUObject, ApplicationCore | Subsystems, Collections |
| **L2 游戏/场景** | — | Engine | Entities (可选), 场景/GameObject |
| **L2 图形抽象** | 002-rendering-rci | RHI, RHICore | SRP Core |
| **L3 渲染管线** | 006-render-pipeline | RenderCore, Renderer (RDG) | URP / HDRP |
| **L3 资源** | 004-resource-system | Engine 内 | Addressables, 内置 |
| **L3 Shader** | 005-shader-system | RenderCore/Renderer | Shader Graph, 内置 |
| **L4 编辑器/UI** | 003-editor-system | UnrealEd, Slate | Editor, uGUI, Input |

---

## 五、三引擎依赖图（并排示意）

```
TenEngine                    Unreal                         Unity
─────────                    ──────                         ─────
Editor(003)                  UnrealEd / Slate               Editor / uGUI / Input
  │  │   │                       │    │                           │
  │  │   └─ Shader(005)          │    └─ Renderer                 ├─ URP/HDRP
  │  │        │                  │         │                     │     │
  │  └─ Resource(004)            │         └─ RenderCore         │     └─ SRP Core
  │        │                     │              │                │
  └─ RenderPipeline(006)          └─ Engine      └─ RHI/RHICore    ├─ Entities.Graphics
           │                        │                │            │     │
           └─ RCI(002)              └─ CoreUObject   └─ Core       ├─ Entities
                  │                       │                        │     │
                  └─ Core(001)            └─ Core                 └─ Engine Core
```

---

## 六、参考链接

- **TenEngine**：`specs/_contracts/000-module-dependency-map.md`、各 spec 的 Dependencies。
- **Unreal**：[Unreal Engine Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)、[Directory Structure](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-directory-structure)、[Render Dependency Graph](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine)。
- **Unity**：[Unity architecture](https://docs.unity3d.com/Manual/unity-architecture.html)、[Packages by keywords](https://docs.unity3d.com/Manual/pack-keys.html)、[Subsystems](https://docs.unity3d.com/Manual/com.unity.modules.subsystems.html)、[URP requirements](https://docs.unity3d.com/Manual/urp/requirements.html)。

以上模块与依赖以公开文档和常见用法为准；具体项目的 Build.cs / 包清单可能略有差异，以实际工程为准。

**TenEngine 整合式提案**：基于本对照设计的更合理模块划分与依赖（L0–L4、RenderCore/PipelineCore、依赖规则与分阶段实施）见 **[proposed-module-architecture.md](./proposed-module-architecture.md)**。
