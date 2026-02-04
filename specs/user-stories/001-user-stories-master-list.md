# 用户故事 · 总览清单（按领域与层级）

本文件为**尽可能完备**的用户故事清单，按**领域**与**层级**组织，便于检索与扩展。单条故事正文见 `domains/<domain>/US-<domain>-<NNN>-<slug>.md`；已实现的故事在对应领域 `index.md` 中列出。

**层级说明**：
- **L0 应用/引擎级**：进程、主循环、模式、配置、插件、平台
- **L1 子系统级**：输入、场景、实体、资源、渲染入口、音频/物理/动画/UI 子系统
- **L2 功能级**：具体功能（射线检测、动画播放、音效播放、UI 点击、网络 RPC 等）
- **L3 工作流级**：编辑器工作流、构建流水线、调试、热重载
- **L4 非功能/质量**：性能、可观测性、错误处理、多线程、可测试性

---

## 1. lifecycle（生命周期）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-lifecycle-001 | 应用启动并进入主循环 | L0 | 已有 |
| US-lifecycle-002 | 用户可选择编辑器启动或游戏模式启动 | L0 | 已有 |
| US-lifecycle-003 | 所有内容分配与现成分配均有统一接口 | L0 | 已有 |
| US-lifecycle-004 | 统一数学库（矩阵/四元数/空间变换全局调用） | L0 | 已有 |
| US-lifecycle-005 | 统一 Check 宏、编译选项控制 Check、容易错误处不用异常；渲染支持 Debug/Hybrid/Resource | L0 | 已有 |
| US-lifecycle-006 | 引擎支持 Android/iOS 等平台、Vulkan/Metal/GLSL/DXIL 等接口、通过宏选择代码路径 | L0 | 已有 |
| US-lifecycle-007 | 应用退出与清理（逆序关闭子系统、释放资源、日志冲刷） | L0 | 待建 |
| US-lifecycle-008 | 启动参数与配置文件（命令行、环境变量、配置文件加载） | L0 | 待建 |
| US-lifecycle-009 | 插件/模块动态加载与卸载 | L0 | 待建 |
| US-lifecycle-010 | 崩溃捕获与错误报告（CheckError、CrashHandler、dump） | L4 | 待建 |

---

## 2. rendering（渲染）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-rendering-001 | 一帧渲染（从场景/相机到屏幕） | L1 | 已有 |
| US-rendering-002 | 流水线式多帧渲染（2–4 帧在途） | L1 | 已有 |
| US-rendering-003 | FrameGraph AddPass 配置 Pass | L1 | 已有 |
| US-rendering-004 | 多线程管线阶段（主循环/游戏更新/收集 RenderItem/逻辑 CB/Device 线程提交） | L1 | 已有 |
| US-rendering-005 | Shader 支持 HLSL/GLSL、宏切换、热重载 | L2 | 已有 |
| US-rendering-006 | 模型渲染 = Mesh + Material 组织，Model 引用 Mesh/Material，Material 保存 Shader/贴图/参数 | L1 | 已有 |
| US-rendering-007 | 渲染资源显式控制位置（CreateRenderItem、CollectCommandBuffer、SubmitCommandBuffer、PrepareRenderMaterial/Mesh、CreateDeviceResource/UpdateDeviceResource） | L1 | 已有 |
| US-rendering-008 | LOD 选择与流式（按距离/屏幕尺寸选择 LOD、按需加载） | L2 | 待建 |
| US-rendering-009 | 视锥剔除与遮挡剔除 | L2 | 待建 |
| US-rendering-010 | 批次与合批（材质/网格批次、实例化绘制） | L2 | 待建 |
| US-rendering-011 | 多视口与多相机（Editor 多视口、分屏、渲染到纹理） | L2 | 待建 |
| US-rendering-012 | 调试绘制（线框、Gizmo、碰撞体预览、DebugDraw） | L3 | 待建 |
| US-rendering-013 | 光照与阴影（方向光、点光、阴影贴图、IBL） | L2 | 待建 |
| US-rendering-014 | 后处理栈（Bloom、DOF、色调映射、抗锯齿） | L2 | 待建 |

---

## 3. resource（资源）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-resource-001 | 异步加载资源并在回调后继续操作 | L1 | 已有 |
| US-resource-002 | 多线程加载、状态与进度、取消 | L1 | 已有 |
| US-resource-003 | 所有资源类型统一 requestLoadAsync（Mesh/Material/Model/Texture/Effect/Terrain/Shader/Audio 等） | L1 | 已有 |
| US-resource-004 | 资源三态（FResource/RResource/DResource）与引用方式 | L1 | 已有 |
| US-resource-005 | 资源卸载与引用计数/GC（显式卸载、依赖解除、GC 策略） | L1 | 待建 |
| US-resource-006 | 流式加载与按需加载（LOD 流式、地形块、大世界） | L2 | 待建 |
| US-resource-007 | 资源依赖解析与预加载（依赖图、预加载子资源） | L2 | 待建 |
| US-resource-008 | 资源导入与格式转换（导入器、元数据、依赖记录） | L2 | 待建 |
| US-resource-009 | 资源包与可寻址（Bundle、Addressables、GUID 寻址） | L2 | 待建 |

---

## 4. input（输入）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-input-001 | 输入轮询与事件（键盘、鼠标、触摸、游戏手柄） | L1 | 待建 |
| US-input-002 | 输入映射与动作（Action/Key 映射、组合键、轴） | L2 | 待建 |
| US-input-003 | 多设备与多平台（设备枚举、平台差异抽象） | L1 | 待建 |
| US-input-004 | 输入与 UI 焦点（焦点获取、输入穿透、编辑器与游戏分离） | L2 | 待建 |

---

## 5. scene（场景）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-scene-001 | 场景加载与切换（加载场景、激活场景、单场景/多场景） | L1 | 待建 |
| US-scene-002 | 场景图与节点（层级、父子、局部/世界变换、脏标记） | L1 | 待建 |
| US-scene-003 | 场景与 Entity 容器（ISceneWorld、createEntity、destroyEntity、getEntities） | L1 | 待建 |
| US-scene-004 | 场景保存与序列化（场景文件、引用解析） | L2 | 待建 |
| US-scene-005 | 子场景与叠加加载（Additive Load、多场景并存） | L2 | 待建 |

---

## 6. entity（实体）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-entity-001 | ECS 组织、Entity 与 Component、快速扩展 Component 类型 | L1 | 已有 |
| US-entity-002 | 实体序列化与预制体（Prefab、序列化/反序列化、引用解析） | L2 | 待建 |
| US-entity-003 | 实体层级与父子（父子关系、局部变换、层级遍历） | L1 | 待建 |
| US-entity-004 | 实体查询与过滤（按 Component 类型查询、按 Tag/Layer 过滤） | L2 | 待建 |
| US-entity-005 | 实体与场景生命周期（随场景加载/卸载创建/销毁） | L1 | 待建 |

---

## 7. editor（编辑器）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-editor-001 | 编辑器内配置渲染设置并保存 | L2 | 已有 |
| US-editor-002 | 编辑器布局（左场景资源管理器、下资源浏览器、右属性面板、中渲染窗口）、点击拾取、拖入、属性面板显示 Component 属性 | L1 | 已有 |
| US-editor-003 | 撤销与重做（Undo/Redo、命令栈、与 Object 序列化联动） | L2 | 待建 |
| US-editor-004 | Play 模式与运行中编辑（进入/退出 Play、运行中暂停、数据隔离） | L2 | 待建 |
| US-editor-005 | 资源导入与预览（导入设置、缩略图、预览窗口） | L2 | 待建 |
| US-editor-006 | 菜单、工具栏与快捷键（主菜单、工具栏、快捷键绑定） | L1 | 待建 |

---

## 8. audio（音频）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-audio-001 | 播放音效（2D 音效、资源加载、播放/停止/循环） | L2 | 待建 |
| US-audio-002 | 3D 音效与混音（空间音效、监听器、混音组、音量） | L2 | 待建 |
| US-audio-003 | 背景音乐与切换（BGM、淡入淡出、切换） | L2 | 待建 |
| US-audio-004 | 音频资源与格式（WAV/OGG、流式、与 Resource 对接） | L1 | 待建 |

---

## 9. physics（物理）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-physics-001 | 射线检测与形状查询（Raycast、Overlap、与 Scene/Entity 对接） | L2 | 待建 |
| US-physics-002 | 刚体与碰撞（RigidBody、Collider、碰撞事件、与 Entity 变换同步） | L2 | 待建 |
| US-physics-003 | 物理场景与步进（物理世界、Step、固定时间步、与 Scene 同步） | L1 | 待建 |
| US-physics-004 | 触发器与碰撞回调（OnTriggerEnter/Exit、OnCollision） | L2 | 待建 |

---

## 10. animation（动画）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-animation-001 | 播放动画剪辑（Clip 播放、播放/暂停/停止、循环、与 Entity 对接） | L2 | 待建 |
| US-animation-002 | 骨骼与蒙皮（Skeleton、蒙皮矩阵、与 Mesh 对接） | L1 | 待建 |
| US-animation-003 | 动画状态机与混合（StateMachine、BlendTree、过渡） | L2 | 待建 |
| US-animation-004 | 动画资源与导入（Clip 资源、骨骼资源、与 Resource 对接） | L1 | 待建 |

---

## 11. ui（UI）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-ui-001 | 画布与控件树（Canvas、控件层级、布局、与 UICore 对接） | L1 | 待建 |
| US-ui-002 | UI 事件与点击（点击、悬停、拖拽、焦点、与 Input 对接） | L2 | 待建 |
| US-ui-003 | UI 样式与主题（样式表、主题、字体、与 UICore 绘制对接） | L2 | 待建 |
| US-ui-004 | UI 与渲染管线（UI Pass、排序、与 Pipeline 对接） | L1 | 待建 |

---

## 12. networking（网络）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-networking-001 | 连接与 RPC（客户端/服务端连接、RPC 调用、与 Application 生命周期对接） | L1 | 待建 |
| US-networking-002 | 实体复制与同步（Replication、属性同步、快照、与 Entity 对接） | L2 | 待建 |
| US-networking-003 | 网络状态与断线重连（连接状态、重连、会话） | L2 | 待建 |

---

## 13. xr（XR）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-xr-001 | XR 会话与帧提交（Session 创建、帧循环、提交到 XR 交换链、与 Pipeline 对接） | L1 | 待建 |
| US-xr-002 | XR 输入与定位（手柄、头显定位、与 Input 对接） | L2 | 待建 |

---

## 14. tools（工具）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-tools-001 | 资源导入与批处理（导入管线、批处理、CLI、与 Resource 对接） | L2 | 待建 |
| US-tools-002 | 构建流水线（打包、Bundle、平台构建、与 Application 对接） | L2 | 待建 |
| US-tools-003 | Shader 编译与 PSO 缓存（离线编译、缓存、与 Shader/RHI 对接） | L2 | 待建 |

---

## 15. 跨领域 / 非功能（L4）

| 编号 | 标题 | 层级 | 状态 |
|------|------|------|------|
| US-lifecycle-010 | 崩溃捕获与错误报告 | L4 | 见 lifecycle |
| US-rendering-012 | 调试绘制 | L3 | 见 rendering |
| （可扩展） | 性能统计与 Profiler（帧时间、GPU 时间、内存） | L4 | 待建 |
| （可扩展） | 日志与追踪（分级日志、文件输出、远程追踪） | L4 | 待建 |
| （可扩展） | 单元测试与集成测试（测试框架、Mock、ABI 稳定性测试） | L4 | 待建 |

---

## 使用说明

- **已有**：故事文档已存在于 `domains/<domain>/US-<domain>-<NNN>-<slug>.md`，并已列入该领域 `index.md`。
- **待建**：建议补充的故事；可按优先级在对应领域下新建 `US-<domain>-<NNN>-<slug>.md`，并更新该领域 `index.md` 与顶层 `000-user-stories-index.md`。
- 单领域 NNN 从 001 起递增，单领域可容纳 999 条；多领域合计可扩展至上万条。
- 新增故事时请遵循 [README § 单条故事格式](./README.md#13-单条故事格式建议) 与 [冲突检测与处理](./README.md#2-冲突检测与处理)。
