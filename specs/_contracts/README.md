# 跨模块接口契约（Contracts）

本目录存放 **TenEngine 各模块之间的接口契约**，供多 Agent 协作时作为接口的单一事实来源。

**契约的官方维护分支**：**`contracts`**（`origin/contracts`）。所有契约更新合并到该分支；其他分支的 Agent 在工作前须从该分支拉取最新契约（`git pull origin contracts` 或 `git fetch origin contracts` + `git merge origin/contracts`）。

## 使用方式

- **在任意特性分支上开始工作前**：先拉取最新契约：`git pull origin contracts`（或 `git fetch origin contracts` 后 `git merge origin/contracts`）。
- **实现某模块前**：阅读本模块 **Dependencies** 中列出的契约文件，只使用契约中声明的类型与接口。
- **修改某模块对外接口时**：在 **`contracts` 分支**上更新本目录下该模块对应的契约，并在 `000-module-dependency-map.md` 中确认下游模块，必要时创建跟进任务。

## 契约列表与依赖关系

- **完整 ABI 实现**：各模块**必须**实现其 ABI 文件（`NNN-modulename-ABI.md`）中列出的**全部**符号与能力；不得仅实现部分或预留「待补充」长期存在。
- **构建须引入子模块代码**：构建过程**必须**通过引入**真实子模块源码**（如 `add_subdirectory`、`FetchContent` 拉取对应模块仓库）来满足依赖；不得在未引入上游模块源码的情况下，用本模块内自写的 stub、mock 或占位实现代替上游模块。
- **禁止 stub/代替方案**：**禁止**为通过编译或联调而提供仅返回空/默认值的 stub 实现、或与 ABI/契约不一致的代替实现作为长期方案；仅允许在**明确标注为临时、且有计划替换为真实实现**的过渡期使用占位，且须在任务/计划中跟踪替换。
- **头文件可见性（IDE）**：构建脚本应将**公开头文件**加入对应 CMake target（如 `target_sources` 或 `add_library` 源列表），并设置 `target_include_directories`，以确保 IDE 工程（VS/Xcode）能正确显示与包含头文件；避免出现“头文件未加入工程或无法 include”的问题。
- 依赖关系总览见 **[000-module-dependency-map.md](./000-module-dependency-map.md)**。

| 契约文件 | 说明 | 提供方 spec | 主要消费者 |
|----------|------|-------------|------------|
| [core-public-api.md](./core-public-api.md) | Core 对外 API（内存、线程、ECS、平台、序列化） | 001-engine-core-module | RCI、Resource、Shader、Editor 等 |
| [rci-public-api.md](./rci-public-api.md) | RCI 渲染抽象层对外 API | 002-rendering-rci-interface | Render Pipeline、Editor、Shader |
| [pipeline-to-rci.md](./pipeline-to-rci.md) | 渲染流水线 → RCI 的命令缓冲与提交约定 | 006-render-pipeline-system | 002-rendering-rci-interface |

## 契约与依赖关系

**本目录内容以 T0-contracts 分支为准**；主分支上的 `_contracts/` 仅作配置或镜像，不作为契约来源。

- **依赖关系总览**：[000-module-dependency-map.md](./000-module-dependency-map.md)（27 模块依赖表与上下游）。
- **ABI 总索引**：[000-module-ABI.md](./000-module-ABI.md)（汇总各模块显式 ABI；引用全部 `NNN-modulename-ABI.md`）。各契约引用本模块对应的 ABI 文件（如 001-core-public-api 引用 [001-core-ABI.md](./001-core-ABI.md)）；接口符号以 ABI 文件为准。从用户故事派生的接口须按 **`docs/engine-abi-interface-generation-spec.md`** 的代码/命名/注释规范生成；用户故事见 **`specs/user-stories/`**。
- **数据相关 TODO**：与资源/资产数据模型、序列化、加载流程、RResource/DResource、GUID→路径等相关的实现项已拆解为 **「数据相关 TODO」** 小节，分布在相关模块的 ABI 文件中。**结构**：按 **注册**、**序列化**、**反序列化**、**数据**、**调用流程** 分类；每类明确 **需提供的接口** 与 **需向上游调用的接口**。**编写原则**：仅描述本模块职责；可引用上游；不引用非上游。依据文档见 **`docs/assets/`**。实现时按各 ABI 内 TODO 逐项完成并勾选。
- **完整依赖图（Mermaid、矩阵、边列表）**：**T0-contracts** 分支下的 **`docs/engine-modules-and-architecture.md`** §4。
- **模块详细规格**：**T0-contracts** 或各 T0-NNN-modulename 分支下的 **`docs/module-specs/`**（001-core.md … 027-xr.md）；各 T0-NNN-modulename 分支仅含对应单模块描述 + 本目录 + 全局依赖图。

## 契约文件列表（T0 架构）

| 契约文件 | 说明 | 提供方模块 | 主要消费者 |
|----------|------|------------|------------|
| [001-core-public-api.md](./001-core-public-api.md) | 001-Core 对外 API（内存、线程、平台、日志、数学、容器、模块加载；无反射/ECS） | 001-Core | 002-Object, 003-Application, 004-Scene, 005-Entity, 006-Input, 007-Subsystems, 008-RHI, 009-RenderCore, 010-Shader, 012-Mesh, 013-Resource, 014-Physics, 015-Animation, 016-Audio, 017-UICore, 020-Pipeline, 022-2D, 023-Terrain, 024-Editor, 026-Networking, 027-XR 等 |
| [002-object-public-api.md](./002-object-public-api.md) | 002-Object 对外 API（反射、序列化、属性、类型注册、GUID） | 002-Object | 004-Scene, 005-Entity, 007-Subsystems, 013-Resource, 015-Animation, 020-Pipeline, 024-Editor 等 |
| [003-application-public-api.md](./003-application-public-api.md) | 003-Application 对外 API（窗口、消息循环、生命周期、DPI） | 003-Application | 004-Scene, 006-Input, 017-UICore, 020-Pipeline, 024-Editor 等 |
| [004-scene-public-api.md](./004-scene-public-api.md) | 004-Scene 对外 API（场景图、节点、空间、加载） | 004-Scene | 005-Entity, 014-Physics, 020-Pipeline, 024-Editor 等 |
| [005-entity-public-api.md](./005-entity-public-api.md) | 005-Entity 对外 API（实体、组件、层级、查询） | 005-Entity | 014-Physics, 015-Animation, 020-Pipeline, 024-Editor, 026-Networking 等 |
| [006-input-public-api.md](./006-input-public-api.md) | 006-Input 对外 API（输入设备、事件、映射） | 006-Input | 017-UICore, 024-Editor, 027-XR 等 |
| [007-subsystems-public-api.md](./007-subsystems-public-api.md) | 007-Subsystems 对外 API（子系统注册、生命周期、查询） | 007-Subsystems | 027-XR 等 |
| [008-rhi-public-api.md](./008-rhi-public-api.md) | 008-RHI 图形抽象层对外 API（RCI 即 RHI；设备、命令列表、资源、PSO、同步） | 008-RHI | 009-RenderCore, 010-Shader, 019-PipelineCore, 020-Pipeline, 024-Editor, 027-XR |
| [009-rendercore-public-api.md](./009-rendercore-public-api.md) | 009-RenderCore 对外 API（Uniform 布局、顶点/Pass 资源描述、UniformBuffer） | 009-RenderCore | 010-Shader, 011-Material, 012-Mesh, 019-PipelineCore, 020-Pipeline, 021-Effects, 022-2D, 023-Terrain |
| [010-shader-public-api.md](./010-shader-public-api.md) | 010-Shader 对外 API（编译、变体、缓存、可选 Graph；Bytecode、Reflection） | 010-Shader | 011-Material, 020-Pipeline, 021-Effects |
| [011-material-public-api.md](./011-material-public-api.md) | 011-Material 对外 API（材质定义、参数、实例、与 Shader/PSO 绑定） | 011-Material | 020-Pipeline |
| [012-mesh-public-api.md](./012-mesh-public-api.md) | 012-Mesh 对外 API（顶点/索引、子网格、LOD、蒙皮） | 012-Mesh | 020-Pipeline, 023-Terrain, 015-Animation |
| [013-resource-public-api.md](./013-resource-public-api.md) | 013-Resource 对外 API（导入、加载、卸载、流式、可寻址；ResourceId、LoadHandle、Metadata） | 013-Resource | 016-Audio, 020-Pipeline, 022-2D, 023-Terrain, 024-Editor |
| [014-physics-public-api.md](./014-physics-public-api.md) | 014-Physics 对外 API（碰撞、刚体、查询、物理场景） | 014-Physics | 022-2D |
| [015-animation-public-api.md](./015-animation-public-api.md) | 015-Animation 对外 API（剪辑、骨骼、播放、蒙皮矩阵） | 015-Animation | 020-Pipeline, 012-Mesh, 024-Editor |
| [016-audio-public-api.md](./016-audio-public-api.md) | 016-Audio 对外 API（音源、监听、混音、空间音效） | 016-Audio | 无（L2 消费端） |
| [017-uicore-public-api.md](./017-uicore-public-api.md) | 017-UICore 对外 API（布局、绘制列表、命中检测、焦点、字体） | 017-UICore | 018-UI, 024-Editor |
| [018-ui-public-api.md](./018-ui-public-api.md) | 018-UI 对外 API（画布、控件树、事件、样式） | 018-UI | 024-Editor |
| [019-pipelinecore-public-api.md](./019-pipelinecore-public-api.md) | 019-PipelineCore 对外 API（Pass 图、资源生命周期、命令格式、提交） | 019-PipelineCore | 020-Pipeline, 021-Effects |
| [020-pipeline-public-api.md](./020-pipeline-public-api.md) | 020-Pipeline 对外 API（管线上下文、渲染目标、DrawCall、可见集/批次） | 020-Pipeline | 021-Effects, 022-2D, 023-Terrain, 024-Editor, 027-XR |
| [pipeline-to-rci.md](./pipeline-to-rci.md) | 020-Pipeline → 008-RHI 的命令缓冲与提交约定（产出方 Pipeline，消费方 RHI） | 边界契约 | 020-Pipeline 与 008-RHI 对接 |
| [021-effects-public-api.md](./021-effects-public-api.md) | 021-Effects 对外 API（后处理栈、粒子、VFX、光照后处理） | 021-Effects | 020-Pipeline, 024-Editor |
| [022-2d-public-api.md](./022-2d-public-api.md) | 022-2D 对外 API（精灵、Tilemap、2D 物理、2D 相机/排序） | 022-2D | 024-Editor |
| [023-terrain-public-api.md](./023-terrain-public-api.md) | 023-Terrain 对外 API（地形、块、LOD、绘制/刷） | 023-Terrain | 024-Editor |
| [024-editor-public-api.md](./024-editor-public-api.md) | 024-Editor 对外 API（视口、场景树、属性、资源浏览器、菜单；无下游） | 024-Editor | 无（L4 消费端） |
| [025-tools-public-api.md](./025-tools-public-api.md) | 025-Tools 对外 API（构建、批处理、CLI、插件/包管理；无下游） | 025-Tools | 无（L4 消费端） |
| [026-networking-public-api.md](./026-networking-public-api.md) | 026-Networking 对外 API（复制、RPC、连接；无下游） | 026-Networking | 无（L4 消费端） |
| [027-xr-public-api.md](./027-xr-public-api.md) | 027-XR 对外 API（XR 会话、帧、提交；无下游） | 027-XR | 无（L4 消费端） |

**说明**：**每模块一契约**，共 27 份模块契约 + 1 份边界契约（pipeline-to-rci）；对应规格见 `docs/module-specs/`。新增或修改契约时请在本 README 和 `000-module-dependency-map.md` 中同步更新。
