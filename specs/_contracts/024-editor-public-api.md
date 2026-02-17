# 契约：024-Editor 模块对外 API

## 适用模块

- **实现方**：024-Editor（L4；视口、场景树、属性面板、资源编辑、菜单；无下游）
- **对应规格**：`docs/module-specs/024-editor.md`
- **依赖**：001-Core、002-Object、003-Application、006-Input、008-RHI、013-Resource、004-Scene、005-Entity、020-Pipeline、018-UI、028-Texture

## 消费者

- 无（L4 消费端）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ViewportHandle | 视口、相机、Gizmo、与 Pipeline/RHI 对接 | 由 Editor 管理 |
| SceneTreeRef | 场景资源管理器；层级视图、选择、拖拽 | 由 Editor 管理 |
| PropertyPanelRef | 属性面板；与 Object 反射联动 | 由 Editor 管理 |
| AssetBrowserRef | 资源浏览器；资源列表、预览、导入 | 由 Editor 管理 |
| MenuRef | 主菜单、工具栏、快捷键 | 由 Editor 管理 |
| GizmoRef | Gizmo 变换工具；平移/旋转/缩放 | 由 Editor 管理 |
| CameraRef | 编辑器相机；飞行/轨道模式 | 由 Editor 管理 |
| SelectionRef | 选择管理器；多选、框选、高亮 | 由 Editor 管理 |
| SnapRef | 对齐设置；网格对齐、旋转对齐 | 由 Editor 管理 |
| ConsoleRef | Console 日志面板；日志级别过滤 | 由 Editor 管理 |
| ProfilerRef | 性能分析器；CPU/GPU 帧时间 | 由 Editor 管理 |
| LayoutRef | 布局管理器；面板布局保存/加载 | 由 Editor 管理 |
| SearchRef | 场景搜索；名称/类型过滤 | 由 Editor 管理 |
| KeyBindingRef | 快捷键系统；快捷键注册和绑定 | 由 Editor 管理 |
| ScriptingRef | 编辑器脚本；命令系统、宏 | 由 Editor 管理 |
| DebugVisRef | 调试可视化；碰撞/导航可视化 | 由 Editor 管理 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 视口 | ViewportHandle、相机、Gizmo、与 Pipeline/RHI 对接；拾取、拖入 |
| 2 | 场景树 | SceneTreeRef、层级、选择、拖拽、与 Scene/Entity 对接 |
| 3 | 属性面板 | PropertyPanelRef、与 Object 反射、编辑、撤销 |
| 4 | 资源浏览器 | AssetBrowserRef、列表、预览、导入、与 Resource 对接 |
| 5 | 菜单 | MenuRef、主菜单、工具栏、快捷键 |
| 6 | Gizmo | GizmoRef、平移/旋转/缩放变换工具 |
| 7 | 编辑器相机 | CameraRef、飞行模式、轨道模式、缩放、Focus |
| 8 | 选择管理 | SelectionRef、多选、框选、选择高亮、选择变化事件 |
| 9 | 对齐设置 | SnapRef、网格对齐、旋转对齐、缩放对齐 |
| 10 | 主菜单 | MenuRef、File/Edit/View/GameObject/Tools/Help 菜单 |
| 11 | 工具栏 | 变换工具切换、Play 控制按钮 |
| 12 | 状态栏 | Level 名称、FPS、选择数量、后台任务 |
| 13 | Console | ConsoleRef、日志显示、级别过滤、搜索 |
| 14 | 偏好设置 | 主题、字体、快捷键映射 |
| 15 | 性能分析 | ProfilerRef、CPU/GPU 帧时间、Draw Call |
| 16 | 场景统计 | Entity 数量、Component 数量 |
| 17 | 布局管理 | LayoutRef、布局保存/加载、预设布局 |
| 18 | Play 模式 | Play/Pause/Stop/Step 控制 |
| 19 | 场景搜索 | SearchRef、名称模糊搜索、类型过滤 |
| 20 | 快捷键系统 | KeyBindingRef、快捷键注册、修改、冲突检测 |
| 21 | 编辑器脚本 | ScriptingRef、命令注册、宏、脚本执行 |
| 22 | 调试可视化 | DebugVisRef、碰撞可视化、导航可视化 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前版本：3.0.0

## 约束

- 须在所有依赖模块初始化之后使用。
- 布局与交互约定（左侧场景树、下方资源浏览器、右侧属性面板等）由实现文档化。
- Play 模式下部分编辑功能受限。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 024-Editor 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
| 2026-02-06 | 新增 P0 能力：Gizmo、Camera、Selection、Snap、Menu、Toolbar、StatusBar、Console、Preferences、Profiler、Statistics、Layout、Play 模式控制 |
| 2026-02-07 | 新增 P1/P2 能力：SceneSearch、KeyBindingSystem、EditorScripting、DebugVisualization |
