# Unity / Unreal 引擎模块划分与依赖参考

本文档整理 Unity、Unreal Engine 的**模块划分**与**依赖说明**，供 TenEngine 架构设计时参考。内容来自官方文档与公开资料，仅作参考，以各引擎最新文档为准。

---

## 一、Unreal Engine

### 1.1 模块架构概览

- **模块**是 UE 软件架构的基本单元，用于封装编辑器工具、运行时功能、库等，以独立代码单元组织。
- 每个项目和插件默认有一个**主模块**，也可在 `Source` 下定义更多模块。
- 依赖通过 **`[ModuleName].Build.cs`** 声明：`PrivateDependencyModuleNames` / `PublicDependencyModuleNames`。
- 模块类型与加载：`.uproject` / `.uplugin` 中的 `Modules` 列表可配置 **Type**（Runtime / Editor 等）、**LoadingPhase**（Default / PreDefault 等）、平台与目标过滤。

**官方文档**：  
[Unreal Engine Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)（多版本可选）

### 1.2 源码目录与模块分类（Engine/Source）

| 分类 | 说明 |
|------|------|
| **Runtime** | 仅运行时使用：Core、引擎运行时、渲染、物理等。 |
| **Developer** | 引擎与编辑器共用：工具、构建、调试等。 |
| **Editor** | 仅编辑器使用：Slate UI、编辑器逻辑等。 |
| **Programs** | 独立程序：UnrealFrontend、UBT 等。 |

常见**运行时核心模块**（与 TenEngine 对应关系可类比）：

- **Core**：基础类型、容器、模块管理（FModuleManager）、平台抽象等。
- **CoreUObject**：UObject、序列化、反射。
- **Engine**：游戏框架、世界、Actor、组件等。
- **RenderCore**：RHI 之上的一层，虚拟纹理、Shader 工具等。
- **Renderer**：高层渲染（Mesh Draw Command、Pass 等）。
- **RHI**（Render Hardware Interface）：图形 API 抽象（D3D12/Vulkan/Metal）。
- **Slate / SlateCore**：UI 框架；Slate 依赖 SlateCore，InputCore 常为 Public 依赖。

模块间通过 **Build.cs** 的 `PublicDependencyModuleNames` / `PrivateDependencyModuleNames` 形成依赖图；只编译依赖链上的模块，并遵循 IWYU（Include What You Use）。

### 1.3 渲染依赖图（RDG）

- **Render Dependency Graph (RDG)**：即时模式 API，将渲染命令记录为图结构，再编译、执行。
- 能力：异步 Compute 调度、瞬态资源分配与生命周期、Split-Barrier 资源状态迁移、并行录制、未使用 Pass/资源剔除、API 与依赖校验、RDG Insights 可视化。
- 管线分为 **Setup**（建图、资源配置）与 **Execute**（执行 Pass Lambda）；Pass 通过参数结构体声明对 RDG 资源的读写，从而推导依赖与生命周期。

**官方文档**：  
[Render Dependency Graph in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine)

### 1.4 小结（UE）

- 模块 = 编译与依赖单元，Build.cs 声明依赖，.uproject/.uplugin 声明加载与平台。
- 分层：Core → CoreUObject → Engine；渲染侧 RHI → RenderCore → Renderer，上层可走 RDG。
- 公开接口放 **Public**，实现放 **Private**；依赖尽量用 Private 以减小编译与耦合。

---

## 二、Unity

### 2.1 架构概览

- 引擎底层为 **Native C/C++**，对外通过 **C#** 脚本与 API 交互。
- 功能以**包（Package）**和**子系统（Subsystems）**组织：包对应功能模块，子系统提供可插拔的运行时能力（Input、Display、XR 等）。

### 2.2 包（Package）与内置模块

- **Package Manager** 管理包；**内置包**多为 `com.unity.modules.*` 形式，可通过勾选/取消控制打包体积。
- 常见**内置模块**（按功能归纳）：
  - **核心/基础**：Accessibility, AI, Animation, Asset Bundle, Audio, Director, Image Conversion, IMGUI, JSONSerialize, Particle System, Physics, Physics 2D, Screen Capture, Terrain
  - **平台**：Android JNI, 等
  - **扩展**：Cloth, Adaptive Performance 等

更多包可通过 **Packages by keywords** 查找：  
[Unity Manual - Packages by keywords](https://docs.unity3d.com/Manual/pack-keys.html)

### 2.3 渲染与图形相关包

- **com.unity.render-pipelines.high-definition**（HDRP）
- **com.unity.render-pipelines.universal**（URP）
- **com.unity.rendering.denoising**
- **com.unity.visualeffectgraph**（VFX Graph）

### 2.4 子系统（Subsystems）

- **com.unity.modules.subsystems**：提供子系统的定义与运行时支持。
- 关键概念：
  - **SubsystemDescriptor**：可在创建实例前查询的元信息。
  - **SubsystemDescriptorStore**：子系统描述符的注册入口。
  - **IntegratedSubsystem**：由描述符创建，提供 Start/Stop 等生命周期，用于 Input、Display、XR 等。
- 子系统可按需启停，便于控制功能与性能。

**参考**：  
[Unity - Manual: Subsystems](https://docs.unity3d.com/Manual/com.unity.modules.subsystems.html)、  
[UnityEngine.SubsystemsModule](https://docs.unity3d.com/ScriptReference/UnityEngine.SubsystemsModule.html)

### 2.5 DOTS / 实体与图形

- **com.unity.entities**：ECS 实体组件系统。
- **com.unity.collections**：高性能容器等。
- **com.unity.entities.graphics**：实体与渲染桥接。
- **com.unity.physics**：Unity Physics（可与 ECS 配合）。

### 2.6 小结（Unity）

- 模块化通过 **Package**（含内置 com.unity.modules.*）和 **Subsystems** 实现；包控制编译与打包，子系统控制运行时能力与依赖。
- 渲染管线以可插拔的 SRP（HDRP/URP）包形式提供，与核心引擎解耦。
- 脚本与架构文档：  
[Unity architecture](https://docs.unity3d.com/Manual/unity-architecture.html)

---

## 三、与 TenEngine 的对照（简要）

| 维度 | Unreal | Unity | TenEngine（当前/目标） |
|------|--------|-------|-------------------------|
| 模块单元 | Module（.Build.cs） | Package + Subsystems | specs/00X-xxx + 契约 |
| 依赖声明 | Build.cs 列表 | 包依赖 + 子系统描述符 | specs/_contracts/ + Dependencies |
| 渲染抽象 | RHI → RenderCore → Renderer，RDG | SRP（HDRP/URP）包 | RCI → Pipeline → CommandBuffer |
| 核心/平台 | Core, CoreUObject, Engine | Native + C#，内置模块 | Core（内存/线程/ECS/平台/序列化） |
| 加载与生命周期 | .uproject LoadingPhase / Type | Subsystem Start/Stop | 契约中的调用顺序与约束 |

---

## 四、参考链接汇总

- **Unreal**
  - [Unreal Engine Modules](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-modules)
  - [Unreal Engine directory structure](https://dev.epicgames.com/documentation/en-us/unreal-engine/unreal-engine-directory-structure)
  - [Render Dependency Graph in Unreal Engine](https://dev.epicgames.com/documentation/en-us/unreal-engine/render-dependency-graph-in-unreal-engine)
- **Unity**
  - [Unity architecture](https://docs.unity3d.com/Manual/unity-architecture.html)
  - [Packages by keywords](https://docs.unity3d.com/Manual/pack-keys.html)
  - [Subsystems (com.unity.modules.subsystems)](https://docs.unity3d.com/Manual/com.unity.modules.subsystems.html)

文档中未给出的具体模块/包列表以官方最新文档为准；TenEngine 的模块与契约以 `specs/_contracts/` 及各 spec 为准。

**详细对照与依赖图**：三引擎（TenEngine、Unreal、Unity）的模块详细列表、依赖表及 ASCII 依赖图见 **[three-engines-modules-and-dependencies.md](./three-engines-modules-and-dependencies.md)**。

**整合式模块划分提案**：基于 Unreal/Unity 设计的更合理模块划分与依赖关系（L0–L4 分层、RenderCore/PipelineCore 拆分、依赖规则与实施阶段）见 **[proposed-module-architecture.md](./proposed-module-architecture.md)**。

**全功能模块规格**：覆盖 Unity + Unreal 全部功能域的完整模块划分（27 模块、功能域映射表、依赖表与依赖图）见 **[tenengine-full-module-spec.md](./tenengine-full-module-spec.md)**。
