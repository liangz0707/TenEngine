# 用户故事 · 顶层索引

本文件为**领域导航**与**冲突/裁决记录**入口；单条故事不在此列举，详见各领域 `domains/<domain>/index.md`。用户故事数量级可上万，按领域分目录组织。

---

## 1. 领域列表与入口

| 领域 | 说明 | 故事数量（示例） | 索引 |
|------|------|------------------|------|
| **lifecycle** | 进程/应用生命周期、主循环、子系统启停、编辑器/游戏模式选择、统一分配接口、统一数学库、统一 Check 宏与编译选项、渲染模式、平台与 RHI 宏选择代码路径、应用退出与清理 | 7 | [domains/lifecycle/index.md](./domains/lifecycle/index.md) |
| **rendering** | 一帧渲染、多帧流水线、FrameGraph AddPass、多线程管线阶段、Shader（HLSL/GLSL、宏、热重载）、模型渲染（Mesh+Material）、渲染资源显式控制位置（CreateRenderItem、CollectCommandBuffer、SubmitCommandBuffer、PrepareRenderMaterial/Mesh、CreateDeviceResource/UpdateDeviceResource）、Pass、Present | 7 | [domains/rendering/index.md](./domains/rendering/index.md) |
| **resource** | 加载/卸载/流式、异步、多线程加载、状态与进度、多种资源类型、资源三态、资源卸载与 GC | 5 | [domains/resource/index.md](./domains/resource/index.md) |
| **input** | 输入轮询与事件（键盘、鼠标、触摸、游戏手柄） | 1 | [domains/input/index.md](./domains/input/index.md) |
| **scene** | 场景加载与切换、场景图与节点（层级、父子、局部/世界变换） | 2 | [domains/scene/index.md](./domains/scene/index.md) |
| **entity** | 实体/组件、ECS、层级、查询、快速扩展 Component 类型、实体序列化与预制体 | 2 | [domains/entity/index.md](./domains/entity/index.md) |
| **editor** | 布局、点击拾取、拖入、属性面板、渲染配置面板、撤销与重做 | 3 | [domains/editor/index.md](./domains/editor/index.md) |
| **audio** | 播放音效（2D、加载、播放/停止/循环） | 1 | [domains/audio/index.md](./domains/audio/index.md) |
| **physics** | 射线检测与形状查询（Raycast、Overlap、与 Scene/Entity 对接） | 1 | [domains/physics/index.md](./domains/physics/index.md) |
| **animation** | 播放动画剪辑（Clip、播放/暂停/循环、与 Entity 对接） | 1 | [domains/animation/index.md](./domains/animation/index.md) |
| **ui** | 画布与控件树（Canvas、控件层级、布局、与 UICore 对接） | 1 | [domains/ui/index.md](./domains/ui/index.md) |
| **networking** | 连接与 RPC（客户端/服务端、RPC 调用） | 1 | [domains/networking/index.md](./domains/networking/index.md) |
| **xr** | XR 会话与帧提交（Session、帧循环、提交到 XR 交换链） | 1 | [domains/xr/index.md](./domains/xr/index.md) |
| **tools** | 资源导入与批处理（导入管线、批处理、CLI） | 1 | [domains/tools/index.md](./domains/tools/index.md) |

**编号约定**：`US-<domain>-<NNN>`，如 `US-lifecycle-001`、`US-rendering-002`。NNN 为 3 位数字（001～999），单领域可容纳 999 条，多领域合计可上万。

---

## 2. 冲突与裁决

当多条用户故事对**同一模块的同一符号或流程**有**不同约定或互斥行为**时，在此记录冲突与人工裁决结果。

### 2.1 冲突记录表（示例）

| 日期 | 故事 A | 故事 B | 冲突描述 | 裁决 |
|------|--------|--------|----------|------|
| （暂无） | — | — | — | — |

### 2.2 冲突类型与处理

- **同符号不同约定**：同一模块、同一符号，签名或语义不一致 → 需人工裁决（采纳其一、合并重载、或拆成两条符号）。
- **同流程互斥**：同一端到端流程，两条故事要求的顺序或分支不可同时满足 → 需人工裁决并更新故事/ABI。
- **资源/生命周期互斥**：对同一资源或对象的创建/销毁/所有权约定矛盾 → 需人工裁决。

详见 [README § 冲突检测与处理](./README.md#2-冲突检测与处理)。

---

## 3. 涉及模块汇总（按领域）

从各领域故事汇总「每个模块参与哪些领域/故事」，便于按模块查看需要提供的能力与接口。具体故事编号见各领域 index。

| 模块 | 参与的领域（示例） | 主要职责摘要 |
|------|--------------------|--------------|
| 001-Core | lifecycle, resource | init/shutdown、InitParams；**引擎支持 Android、iOS 等平台**；**可通过宏选择平台代码路径**（TE_PLATFORM_ANDROID、TE_PLATFORM_IOS 等）；**统一分配接口** IAllocator；**统一数学库** TenEngine::math；**统一 Check 宏** CheckWarning()、CheckError()；**可通过编译选项设置启用的 Check**；IThreadPool、submitTask、getThreadPool |
| 003-Application | lifecycle | 窗口、主循环、RunParams、TickCallback、requestQuit；RunMode（Editor/Game）、getRunMode |
| 007-Subsystems | lifecycle | ISubsystem、SubsystemRegistry、initAll、shutdownAll、getSubsystem<T>；Editor 作为子系统注册 |
| 024-Editor | lifecycle, editor | **布局**：左场景资源管理器、下资源浏览器、右属性面板、中渲染窗口；getSceneView、getResourceView、getPropertyPanel、getRenderViewport；渲染窗口**点击拾取**、**从资源管理器拖入**；右侧属性面板**显示各种 Component 的属性**；getRenderingSettingsPanel、IRenderingSettingsPanel、RenderingConfig、saveConfig/loadConfig |
| 020-Pipeline | rendering, editor | **渲染资源显式控制位置**：SubmitCommandBuffer（submitLogicalCommandBuffer）；**渲染支持 Debug、Hybrid、Resource**；renderFrame、tickPipeline、getCurrentSlot、FrameContext、RenderPipelineDesc；setRenderingConfig、getRenderingConfig、RenderingConfig；getFrameGraph、setFrameGraph、createFrameGraph；triggerRender、submitLogicalCommandBuffer |
| 021-Effects | rendering, editor | 后处理与抗锯齿按 RenderingConfig 生效（Pipeline 下发）；可选监听 Shader 更新并刷新 PSO |
| 008-RHI | rendering | **渲染资源显式控制位置**：CreateDeviceResource、UpdateDeviceResource；SubmitCommandBuffer（executeLogicalCommandBuffer、submitCommandList）；支持 Vulkan/Metal/D3D12 及 GLSL/HLSL/DXIL/MSL；**可通过宏选择后端代码路径**；IDevice、createDevice、ICommandList、ISwapChain；getCommandListForSlot、submitCommandList、waitForSlot、Present 同步；executeLogicalCommandBuffer（Device 线程执行逻辑 CB 并提交） |
| 019-PipelineCore | rendering | **渲染资源显式控制位置**：CreateRenderItem、CollectCommandBuffer（convertToLogicalCommandBuffer）、PrepareRenderMaterial、PrepareRenderMesh、prepareRenderResources；IFrameGraph、addPass、buildLogicalPipeline、RenderItem、IRenderItemList、collectRenderItemsParallel、mergeRenderItems、ILogicalCommandBuffer、convertToLogicalCommandBuffer |
| 010-Shader | rendering | ShaderSourceFormat（HLSL/GLSL）、loadSource、compile、getBytecode；MacroSet、VariantKey、setMacros、selectVariant（游戏中动态切换宏）；reloadShader、onSourceChanged、notifyShaderUpdated（Shader 热重载/实时更新）；Cache、defineKeyword、enumerateVariants、precompile |
| 011-Material | rendering, editor | **引擎格式**材质；**保存 Shader**，引用贴图、材质参数；材质定义、参数、实例、与 Shader/PSO 绑定；可选监听 Shader 更新并刷新 PSO |
| 012-Mesh | rendering | Mesh 来源于 **OBJ、FBX** 等常用格式；顶点/索引、子网格、LOD；可单独加载或经 Model 引用 |
| 002-Object | entity | registerType、getTypeInfo、ITypeInfo、TypeId；供 Component 类型快速扩展 |
| 004-Scene | entity | ISceneWorld、createEntity、destroyEntity、getEntities、getSceneWorld |
| 005-Entity | entity | IEntity、addComponent、getComponent、removeComponent、hasComponent；TransformComponent、ModelComponent、ScriptComponent、EffectComponent、DecalComponent、TerrainComponent、LightComponent；IComponentRegistry、registerComponentType、getComponentRegistry |
| 013-Resource | resource, rendering | **资源三态**：**FResource**（硬盘，GUID 引用）、**RResource**（内存，指针引用，**DResource 保存在 RResource 内部**）、**DResource**（GPU）；硬盘加载使用 FResource，内存引用使用 RResource；**统一** requestLoadAsync；Model 引用 Mesh/Material；IModelResource、IMeshResource、IMaterialResource、getLoadStatus、getLoadProgress、cancelLoad、IResource、getResourceType、ResourceType；ITextureResource、IEffectResource、ITerrainResource、registerResourceLoader、LoadResult、getResourceManager |

*随领域与故事增加，本表由人工或脚本从各领域 index 汇总更新。*

---

## 4. 与 ABI 的衔接

- 每个故事拆解后，**各模块需要的类型、函数签名、回调/事件** 按 **`docs/engine-abi-interface-generation-spec.md`** 写成 ABI 表行。
- 定稿条目写入 **`specs/_contracts/NNN-modulename-ABI.md`**；总索引见 **`specs/_contracts/000-module-ABI.md`**。
- 发现**冲突**时：暂停写入、在「冲突与裁决」中记录、经人工裁决后再更新 ABI 与故事。

---

## 5. 旧编号对照（扁平 US-001～004 已迁至领域）

| 旧编号 | 新编号 | 领域 |
|--------|--------|------|
| US-001 | US-lifecycle-001 | [lifecycle](./domains/lifecycle/index.md) |
| US-002 | US-rendering-001 | [rendering](./domains/rendering/index.md) |
| US-003 | US-resource-001 | [resource](./domains/resource/index.md) |
| US-004 | US-rendering-002 | [rendering](./domains/rendering/index.md) |

根目录下原 `US-001-…`～`US-004-…` 文件保留为**重定向说明**，正式内容以 `domains/` 下为准。
