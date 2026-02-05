# TenEngine 完整模块与基础架构

本文档描述 TenEngine 的**完整模块划分、层级与依赖关系**。模块边界清晰、依赖单向、可并行开发。**依赖的单一形式**：**§三、依赖边列表**（便于脚本/工具解析）；§二为依赖表与规则。

---

## 〇、概述

- **29 模块**，按 **L0～L4** 分层；L0 基础无业务依赖，L4 工具与扩展可依赖所有运行时模块。场景拆分为 **004-Scene**（逻辑场景管理，L1）与 **029-World**（场景资源管理 / Level 实际引用，L2）。
- **渲染链单向**：RHI ← RenderCore ← PipelineCore ← Pipeline；Resource 与渲染解耦。
- **契约**：每模块对应 `specs/_contracts/NNN-modulename-public-api.md`；边界契约 `pipeline-to-rci`。详见 **五、契约文件建议**。
- **详细规格**：各模块的详细功能、实现难度、子模块、上下游见 **`docs/module-specs/`**（如 `001-core.md` … `029-world.md`）。

---

## 一、模块列表与层级

### 1.1 层级定义

- **L0 基础**：无业务依赖；所有模块最终依赖于此。
- **L1 平台/场景/抽象**：依赖 L0；应用、场景、实体、输入、子系统、RHI。
- **L2 内容与能力**：依赖 L0–L1；渲染类型、Shader、网格、资源、物理、动画、音频、UI。
- **L3 管线与特性**：依赖 L0–L2；管线协议、管线实现、特效、2D、地形。
- **L4 工具与扩展**：依赖 L0–L3；编辑器、工具、网络、XR。

### 1.2 模块清单（按层级）

#### L0 基础层

| 编号 | 模块名 | 功能 | 直接依赖 |
|------|--------|------|----------|
| 001 | **Core** | 内存、线程、平台（文件/时间/环境）、日志、数学、容器、模块加载；无反射无 ECS | — |
| 002 | **Object** | 反射、序列化、属性系统、类型注册 | Core |
| 003 | **Application** | 应用生命周期、窗口、消息循环、主循环 | Core |

#### L1 场景 / 实体 / 输入 / 图形抽象

| 编号 | 模块名 | 功能 | 直接依赖 |
|------|--------|------|----------|
| 004 | **Scene** | 场景图、层级、World/Level 容器、激活/禁用（逻辑场景管理，不持有资源） | Core, Object |
| 005 | **Entity** | 实体/组件模型或 ECS | Core, Object, Scene |
| 006 | **Input** | 输入抽象、键鼠/手柄/触摸、原始输入 | Core, Application |
| 007 | **Subsystems** | 可插拔子系统：描述符、注册、Start/Stop（Display、XR 等） | Core, Object |
| 008 | **RHI** | 图形 API 抽象（Vulkan/D3D12/Metal）；命令列表、资源、PSO、多后端 | Core |

#### L2 渲染类型 / 内容 / 物理 / 动画 / 音频 / UI

| 编号 | 模块名 | 功能 | 直接依赖 |
|------|--------|------|----------|
| 009 | **RenderCore** | Shader 参数结构、渲染资源描述、Pass 参数协议、Uniform Buffer | Core, RHI |
| 010 | **Shader** | Shader 编译、变体、预编译、可选 Shader Graph 式编辑 | Core, RHI, RenderCore, Resource |
| 011 | **Material** | 材质定义、参数、与 Shader 绑定、材质实例 | RenderCore, Shader, Texture, Resource |
| 012 | **Mesh** | 网格数据、LOD、蒙皮、顶点/索引（EnsureDeviceResources 时依赖 RHI 创建缓冲） | Core, RHI, RenderCore, Resource |
| 028 | **Texture** | 贴图数据、格式、Mipmap、与 RHI 纹理资源对接 | Core, RHI, RenderCore, Resource |
| 013 | **Resource** | 资源导入、同步/异步加载、卸载、流式、可寻址 | Core, Object, Texture |
| 029 | **World** | 场景资源管理、Level 实际资源引用；底层依赖 004-Scene 做场景管理；013 Load(level) 后由 World 调 004 CreateSceneFromDesc 并持有关卡句柄 | Scene, Resource |
| 014 | **Physics** | 碰撞、刚体、查询、2D/3D | Core, Scene, Entity |
| 015 | **Animation** | 动画剪辑、骨骼动画、Timeline、状态机 | Core, Object, Entity |
| 016 | **Audio** | 音源、监听、混音、空间音效 | Core, Resource |
| 017 | **UICore** | UI 布局、绘制、输入路由 | Core, Application, Input |
| 018 | **UI** | 控件、画布、事件 | UICore |

#### L3 管线与特性

| 编号 | 模块名 | 功能 | 直接依赖 |
|------|--------|------|----------|
| 019 | **PipelineCore** | 命令缓冲格式、Pass 图协议（RDG 风格）、与 RHI 提交约定 | RHI, RenderCore |
| 020 | **Pipeline** | 场景收集、剔除、DrawCall、命令缓冲生成、提交；Animation（可选） | Core, Scene, Entity, PipelineCore, RenderCore, Shader, Material, Mesh, Texture, Resource, Effects；Animation（可选） |
| 021 | **Effects** | 后处理、粒子/VFX、光照后处理 | PipelineCore, RenderCore, Shader, Texture |
| 022 | **2D** | 精灵、Tilemap、2D 物理、2D 渲染 | Core, Resource, Physics, Pipeline, RenderCore, Texture |
| 023 | **Terrain** | 地形数据、LOD、绘制/刷 | Core, Resource, Mesh, Pipeline, RenderCore, Texture |

#### L4 工具与扩展

| 编号 | 模块名 | 功能 | 直接依赖 |
|------|--------|------|----------|
| 024 | **Editor** | 视口、场景树、属性面板、资源编辑、菜单 | Core, Application, Input, RHI, Resource, Scene, Entity, Pipeline, UI, Texture |
| 025 | **Tools** | 构建、批处理、CLI、插件/包管理 | 按需 |
| 026 | **Networking** | 复制、RPC、客户端/服务端 | Core, Entity |
| 027 | **XR** | AR/VR 子系统、头显、手柄 | Core, Subsystems, Input, Pipeline |

---

## 二、依赖关系总表与规则

### 2.1 下游 → 上游（直接依赖）

| 模块 | 直接依赖 |
|------|----------|
| 001-Core | — |
| 002-Object | Core |
| 003-Application | Core |
| 004-Scene | Core, Object |
| 005-Entity | Core, Object, Scene |
| 006-Input | Core, Application |
| 007-Subsystems | Core, Object |
| 008-RHI | Core |
| 009-RenderCore | Core, RHI |
| 010-Shader | Core, RHI, RenderCore, Resource |
| 011-Material | RenderCore, Shader, Texture, Resource |
| 012-Mesh | Core, RHI, RenderCore, Resource |
| 028-Texture | Core, RHI, RenderCore, Resource |
| 013-Resource | Core, Object, Texture |
| 029-World | Scene, Resource |
| 014-Physics | Core, Scene, Entity |
| 015-Animation | Core, Object, Entity |
| 016-Audio | Core, Resource |
| 017-UICore | Core, Application, Input |
| 018-UI | UICore |
| 019-PipelineCore | RHI, RenderCore |
| 020-Pipeline | Core, Scene, Entity, PipelineCore, RenderCore, Shader, Material, Mesh, Texture, Resource, Effects；Animation（可选） |
| 021-Effects | PipelineCore, RenderCore, Shader, Texture |
| 022-2D | Core, Resource, Physics, Pipeline, RenderCore, Texture |
| 023-Terrain | Core, Resource, Mesh, Pipeline, RenderCore, Texture |
| 024-Editor | Core, Application, Input, RHI, Resource, Scene, Entity, Pipeline, UI, Texture |
| 025-Tools | 按需 |
| 026-Networking | Core, Entity |
| 027-XR | Core, Subsystems, Input, Pipeline |

### 2.2 依赖规则（强制）

1. **L0 无依赖**：Core 无依赖；Object、Application 仅依赖 Core。
2. **L1 仅依赖 L0**：Scene、Entity、Input、Subsystems、RHI 不依赖 L2–L4。
3. **L2 仅依赖 L0–L2（不依赖 L3–L4）**：RenderCore、Shader、Material、Mesh、Texture、Resource、**World**、Physics、Animation、Audio、UICore、UI 不依赖 L3–L4；L2 内可同层互依（如 011 依赖 010、028，013 依赖 028）。
4. **L3 仅依赖 L0–L2**：PipelineCore、Pipeline、Effects、2D、Terrain 不依赖 L4。
5. **L4 可依赖 L0–L3**：Editor、Tools、Networking、XR 可依赖所有运行时模块；运行时模块不依赖 L4。
6. **渲染链单向**：RHI ← RenderCore ← PipelineCore ← Pipeline；Pipeline 产出命令缓冲，RHI 执行。
7. **Resource 与渲染解耦**：Resource 不依赖 RHI/RenderCore/Pipeline；管线通过资源句柄/描述使用资源。
8. **设备层资源须依赖 RHI**：凡需**创建或持有设备层资源**（如 GPU 缓冲、纹理、PSO、命令列表）的模块，须**直接依赖 008-RHI** 进行创建；当前包括 009-RenderCore、010-Shader、012-Mesh、028-Texture、019-PipelineCore、020-Pipeline（通过 RHI 提交）、024-Editor（视口/交换链）等。

若依赖有变更，请同步更新 **`specs/_contracts/000-module-dependency-map.md`** 与本文档。

---

## 三、依赖边列表

每行一条边：`下游模块 直接依赖 上游模块`（便于脚本/工具解析）。

```
002-Object        001-Core
003-Application  001-Core
004-Scene        001-Core 002-Object
005-Entity       001-Core 002-Object 004-Scene
006-Input        001-Core 003-Application
007-Subsystems   001-Core 002-Object
008-RHI          001-Core
009-RenderCore   001-Core 008-RHI
010-Shader       001-Core 008-RHI 009-RenderCore 013-Resource
011-Material     009-RenderCore 010-Shader 028-Texture 013-Resource
012-Mesh         001-Core 008-RHI 009-RenderCore 013-Resource
028-Texture      001-Core 008-RHI 009-RenderCore 013-Resource
013-Resource     001-Core 002-Object 028-Texture
029-World        004-Scene 013-Resource
014-Physics      001-Core 004-Scene 005-Entity
015-Animation    001-Core 002-Object 005-Entity
016-Audio        001-Core 013-Resource
017-UICore       001-Core 003-Application 006-Input
018-UI           017-UICore
019-PipelineCore 008-RHI 009-RenderCore
020-Pipeline     001-Core 004-Scene 005-Entity 019-PipelineCore 009-RenderCore 010-Shader 011-Material 012-Mesh 028-Texture 013-Resource 021-Effects（015-Animation 可选）
021-Effects      019-PipelineCore 009-RenderCore 010-Shader 028-Texture
022-2D           001-Core 013-Resource 014-Physics 020-Pipeline 009-RenderCore 028-Texture
023-Terrain      001-Core 013-Resource 012-Mesh 020-Pipeline 009-RenderCore 028-Texture
024-Editor       001-Core 003-Application 006-Input 008-RHI 013-Resource 004-Scene 005-Entity 020-Pipeline 018-UI 028-Texture
026-Networking   001-Core 005-Entity
027-XR           001-Core 007-Subsystems 006-Input 020-Pipeline
```

---

## 四、可选模块与最小集

- **必选（最小可运行）**：Core, Object, Application, Scene, Entity, RHI, RenderCore, Shader, Material, Mesh, Resource, PipelineCore, Pipeline。
- **可选按需**：Input, Subsystems, Physics, Animation, Audio, UICore, UI, Effects, 2D, Terrain, Editor, Tools, Networking, XR。
- 实施时可先实现 L0 + L1（含 RHI）+ L2（RenderCore, Shader, Material, Mesh, Resource）+ L3（PipelineCore, Pipeline），再按需增加 2D、Terrain、Effects、Editor 等。

---

## 五、契约文件建议（与模块对应）

| 模块 | 建议契约 |
|------|----------|
| 001-Core | 001-core-public-api |
| 002-Object | 002-object-public-api |
| 003-Application | 003-application-public-api |
| 004-Scene | 004-scene-public-api |
| 005-Entity | 005-entity-public-api |
| 006-Input | 006-input-public-api |
| 007-Subsystems | 007-subsystems-public-api |
| 008-RHI | 008-rhi-public-api |
| 009-RenderCore | 009-rendercore-public-api |
| 010-Shader | 010-shader-public-api |
| 011-Material | 011-material-public-api |
| 012-Mesh | 012-mesh-public-api |
| 028-Texture | 028-texture-public-api |
| 013-Resource | 013-resource-public-api |
| 029-World | 029-world-public-api |
| 014-Physics | 014-physics-public-api |
| 015-Animation | 015-animation-public-api |
| 016-Audio | 016-audio-public-api |
| 017-UICore | 017-uicore-public-api |
| 018-UI | 018-ui-public-api |
| 019-PipelineCore | 019-pipelinecore-public-api |
| 020-Pipeline | 020-pipeline-public-api；与 008 边界见 pipeline-to-rci |
| 021-Effects | 021-effects-public-api |
| 022-2D | 022-2d-public-api |
| 023-Terrain | 023-terrain-public-api |
| 024-Editor | 024-editor-public-api |
| 025-Tools | 025-tools-public-api（按需） |
| 026-Networking | 026-networking-public-api |
| 027-XR | 027-xr-public-api |

契约文件位于 `specs/_contracts/`，命名 `NNN-modulename-public-api.md`。边界契约 `pipeline-to-rci` 见同目录。

---

## 六、与现有 TenEngine 规格的对应

| 现有 spec | 本规格模块 | 说明 |
|-----------|------------|------|
| 001-engine-core-module | 001-Core + 002-Object + 003-Application | 拆分为纯基础、对象、应用 |
| 002-rendering-rci-interface | 008-RHI | 一致 |
| 003-editor-system | 024-Editor + 017-UICore + 018-UI | 编辑器 + UI 拆分 |
| 004-resource-system | 013-Resource | 一致 |
| 005-shader-system | 010-Shader + 011-Material | Shader 与 Material 可分可合 |
| 006-render-pipeline-system | 019-PipelineCore + 020-Pipeline | 协议与实现拆分 |
| 006-thirdparty-integration-tool | 025-Tools | 一致 |
| — | 004–007, 009–012, 014–016, 021–023, 026–029 | 新增模块 |

本规格为**目标架构**；现有 `specs/001–006` 可保留并逐步向本表靠拢，或按本表新建 spec 与契约。

**按模块的详细规格**（含模块简要说明、详细功能、实现难度、资源类型、子模块、上下游与外部依赖）见 **`docs/module-specs/`**，文件名如 `001-core.md` … `029-world.md`。

