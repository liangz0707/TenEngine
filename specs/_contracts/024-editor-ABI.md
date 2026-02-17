# 024-Editor 模块 ABI

- **契约**：[024-editor-public-api.md](./024-editor-public-api.md)（能力与类型描述）
- **本文件**：024-Editor 对外 ABI 显式表。
- **布局约定**：**左侧**场景资源管理器、**下方**资源浏览器、**右侧**属性面板、**中间**渲染窗口；渲染窗口中模型可**点击拾取**，可从资源管理器**拖入**；右侧属性面板可**显示各种 Component 的属性**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 核心接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IEditor | 抽象接口 | 编辑器主入口 | te/editor/Editor.h | IEditor::Run | 进入编辑器主循环 |
| 024-Editor | te::editor | — | struct | 编辑器上下文 | te/editor/Editor.h | EditorContext | 窗口句柄、projectRootPath、application、resourceManager |
| 024-Editor | te::editor | — | 自由函数/工厂 | 创建编辑器实例 | te/editor/Editor.h | CreateEditor | `IEditor* CreateEditor(EditorContext const& ctx);` |
| 024-Editor | te::editor | — | struct | 编辑器类型定义 | te/editor/EditorTypes.h | EditorTypes | GizmoMode、PlayModeState、ViewportMode 等枚举和结构体 |

### 现有面板接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IEditor | 抽象接口 | 场景资源管理器 | te/editor/SceneView.h | IEditor::GetSceneView | 场景层级、Entity 树 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 资源浏览器 | te/editor/ResourceView.h | IEditor::GetResourceView | 资源列表、预览、导入 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 属性面板 | te/editor/PropertyPanel.h | IEditor::GetPropertyPanel | 显示 Component 属性 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 渲染窗口 | te/editor/Viewport.h | IEditor::GetRenderViewport | 视口渲染、拾取、拖入 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 渲染设置面板 | te/editor/Editor.h | IEditor::GetRenderingSettingsPanel | 渲染配置面板 |

### P0 核心组件接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IGizmo | 抽象接口 | Gizmo 变换工具 | te/editor/Gizmo.h | IEditor::GetGizmo | 平移/旋转/缩放 Gizmo |
| 024-Editor | te::editor | IEditorCamera | 抽象接口 | 编辑器相机 | te/editor/EditorCamera.h | IEditor::GetEditorCamera | 飞行/轨道模式相机 |
| 024-Editor | te::editor | ISelectionManager | 抽象接口 | 选择管理器 | te/editor/SelectionManager.h | IEditor::GetSelectionManager | 多选、框选、高亮 |
| 024-Editor | te::editor | ISnapSettings | 抽象接口 | 对齐设置 | te/editor/SnapSettings.h | IEditor::GetSnapSettings | 网格/旋转/缩放对齐 |
| 024-Editor | te::editor | IMainMenu | 抽象接口 | 主菜单 | te/editor/MainMenu.h | IEditor::GetMainMenu | File/Edit/View/GameObject 菜单 |
| 024-Editor | te::editor | IToolbar | 抽象接口 | 工具栏 | te/editor/Toolbar.h | IEditor::GetToolbar | 变换工具、Play 控制 |
| 024-Editor | te::editor | IStatusBar | 抽象接口 | 状态栏 | te/editor/StatusBar.h | IEditor::GetStatusBar | FPS、Level 名称、选择数 |
| 024-Editor | te::editor | IConsolePanel | 抽象接口 | Console 面板 | te/editor/ConsolePanel.h | IEditor::GetConsolePanel | 日志显示、过滤 |
| 024-Editor | te::editor | IEditorPreferences | 抽象接口 | 编辑器偏好 | te/editor/EditorPreferences.h | IEditor::GetPreferences | 主题、快捷键、设置 |
| 024-Editor | te::editor | IProfilerPanel | 抽象接口 | 性能分析器 | te/editor/ProfilerPanel.h | IEditor::GetProfilerPanel | CPU/GPU 帧时间 |
| 024-Editor | te::editor | IStatisticsPanel | 抽象接口 | 场景统计 | te/editor/StatisticsPanel.h | IEditor::GetStatisticsPanel | Entity/Component 数量 |
| 024-Editor | te::editor | ILayoutManager | 抽象接口 | 布局管理 | te/editor/LayoutManager.h | IEditor::GetLayoutManager | 面板布局保存/加载 |

### P1 新增组件接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | ISceneSearch | 抽象接口 | 场景搜索 | te/editor/SceneSearch.h | CreateSceneSearch | 名称/类型搜索过滤 |
| 024-Editor | te::editor | IKeyBindingSystem | 抽象接口 | 快捷键系统 | te/editor/KeyBindingSystem.h | CreateKeyBindingSystem | 快捷键注册、绑定、执行 |
| 024-Editor | te::editor | IEditorScripting | 抽象接口 | 编辑器脚本 | te/editor/EditorScripting.h | CreateEditorScripting | 命令注册、宏、脚本执行 |

### P2 调试工具接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IDebugVisualization | 抽象接口 | 调试可视化 | te/editor/DebugVisualization.h | CreateDebugVisualization | 碰撞/导航/Bounds 可视化 |

### P3 高级功能接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IPrefabSystem | 抽象接口 | Prefab 系统 | te/editor/PrefabSystem.h | CreatePrefabSystem | Prefab 创建、实例化、覆盖 |
| 024-Editor | te::editor | IPluginSystem | 抽象接口 | 插件系统 | te/editor/PluginSystem.h | CreatePluginSystem | 插件加载、Hook、菜单 |
| 024-Editor | te::editor | IHistoryManager | 抽象接口 | 历史管理 | te/editor/HistoryManager.h | CreateHistoryManager | 增强 Undo/Redo、书签 |
| 024-Editor | te::editor | IMultiObjectEditor | 抽象接口 | 多对象编辑 | te/editor/MultiObjectEditing.h | CreateMultiObjectEditor | 多选编辑、对齐、分布 |
| 024-Editor | te::editor | IViewportStats | 抽象接口 | 视口统计 | te/editor/ViewportStats.h | CreateViewportStats | FPS、Draw Call、内存统计 |

### Play 模式控制接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IEditor | 抽象接口 | 进入 Play 模式 | te/editor/Editor.h | IEditor::EnterPlayMode | 开始运行游戏 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 暂停 Play 模式 | te/editor/Editor.h | IEditor::PausePlayMode | 暂停游戏 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 停止 Play 模式 | te/editor/Editor.h | IEditor::StopPlayMode | 停止并返回编辑模式 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 单帧步进 | te/editor/Editor.h | IEditor::StepFrame | 前进一帧 |
| 024-Editor | te::editor | IEditor | 抽象接口 | Play 模式状态 | te/editor/Editor.h | IEditor::IsInPlayMode | 查询当前状态 |

### 布局管理接口

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IEditor | 抽象接口 | 保存布局 | te/editor/Editor.h | IEditor::SaveLayout | 保存面板布局 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 加载布局 | te/editor/Editor.h | IEditor::LoadLayout | 加载面板布局 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 重置布局 | te/editor/Editor.h | IEditor::ResetLayout | 重置默认布局 |

## 工厂函数

| 函数签名 | 头文件 | 说明 |
|----------|--------|------|
| `IEditor* CreateEditor(EditorContext const& ctx);` | te/editor/Editor.h | 创建编辑器实例 |
| `IGizmo* CreateGizmo();` | te/editor/Gizmo.h | 创建 Gizmo 实例 |
| `IEditorCamera* CreateEditorCamera();` | te/editor/EditorCamera.h | 创建编辑器相机 |
| `ISelectionManager* CreateSelectionManager();` | te/editor/SelectionManager.h | 创建选择管理器 |
| `ISnapSettings* CreateSnapSettings();` | te/editor/SnapSettings.h | 创建对齐设置 |
| `IMainMenu* CreateMainMenu();` | te/editor/MainMenu.h | 创建主菜单 |
| `IToolbar* CreateToolbar();` | te/editor/Toolbar.h | 创建工具栏 |
| `IStatusBar* CreateStatusBar();` | te/editor/StatusBar.h | 创建状态栏 |
| `IConsolePanel* CreateConsolePanel();` | te/editor/ConsolePanel.h | 创建 Console 面板 |
| `IEditorPreferences* CreateEditorPreferences();` | te/editor/EditorPreferences.h | 创建编辑器偏好 |
| `IProfilerPanel* CreateProfilerPanel();` | te/editor/ProfilerPanel.h | 创建性能分析器 |
| `IStatisticsPanel* CreateStatisticsPanel();` | te/editor/StatisticsPanel.h | 创建统计面板 |
| `ILayoutManager* CreateLayoutManager();` | te/editor/LayoutManager.h | 创建布局管理器 |
| `ISceneSearch* CreateSceneSearch();` | te/editor/SceneSearch.h | 创建场景搜索 |
| `IKeyBindingSystem* CreateKeyBindingSystem();` | te/editor/KeyBindingSystem.h | 创建快捷键系统 |
| `IEditorScripting* CreateEditorScripting();` | te/editor/EditorScripting.h | 创建脚本系统 |
| `IDebugVisualization* CreateDebugVisualization();` | te/editor/DebugVisualization.h | 创建调试可视化 |
| `IPrefabSystem* CreatePrefabSystem();` | te/editor/PrefabSystem.h | 创建 Prefab 系统 |
| `IPluginSystem* CreatePluginSystem();` | te/editor/PluginSystem.h | 创建插件系统 |
| `IHistoryManager* CreateHistoryManager();` | te/editor/HistoryManager.h | 创建历史管理器 |
| `IMultiObjectEditor* CreateMultiObjectEditor();` | te/editor/MultiObjectEditing.h | 创建多对象编辑器 |
| `IViewportStats* CreateViewportStats();` | te/editor/ViewportStats.h | 创建视口统计 |

## 文件结构

```
Engine/TenEngine-024-editor/
├── include/te/editor/
│   ├── Editor.h              # 主接口
│   ├── EditorTypes.h         # 类型定义
│   ├── EditorPanel.h         # 面板基类
│   ├── Gizmo.h               # Gizmo 变换工具
│   ├── EditorCamera.h        # 编辑器相机
│   ├── SelectionManager.h    # 选择管理
│   ├── SnapSettings.h        # 对齐设置
│   ├── MainMenu.h            # 主菜单
│   ├── Toolbar.h             # 工具栏
│   ├── StatusBar.h           # 状态栏
│   ├── ConsolePanel.h        # Console 面板
│   ├── EditorPreferences.h   # 编辑器偏好
│   ├── ProfilerPanel.h       # 性能分析器
│   ├── StatisticsPanel.h     # 场景统计
│   ├── LayoutManager.h       # 布局管理
│   ├── SceneSearch.h         # 场景搜索
│   ├── KeyBindingSystem.h    # 快捷键系统
│   ├── EditorScripting.h     # 编辑器脚本
│   ├── DebugVisualization.h  # 调试可视化
│   ├── PrefabSystem.h        # Prefab 系统
│   ├── PluginSystem.h        # 插件系统
│   ├── HistoryManager.h      # 历史管理
│   ├── MultiObjectEditing.h  # 多对象编辑
│   ├── ViewportStats.h       # 视口统计
│   ├── Viewport.h            # 视口接口
│   ├── SceneView.h           # 场景树
│   ├── ResourceView.h        # 资源浏览器
│   ├── PropertyPanel.h       # 属性面板
│   ├── UndoSystem.h          # 撤销系统
│   └── ...                   # 其他文件
└── src/
    └── [对应实现文件]
```

## 版本历史

| 版本 | 日期 | 变更说明 |
|------|------|----------|
| 3.0.0 | 2026-02-07 | 新增 P3 高级功能：Prefab、Plugin、History、MultiObject、ViewportStats |
| 2.0.0 | 2026-02-06 | 新增 P1-P2 功能：SceneSearch、KeyBinding、Scripting、DebugVis |
| 1.0.0 | 2026-02-05 | 初始版本：核心面板 + P0 组件 |

*来源：用户故事 US-lifecycle-002、US-editor-001~003；契约能力：PickInViewport、DropFromResourceManager、UndoRedo、Gizmo、Camera、Selection、PlayMode、KeyBinding、DebugVis、Prefab、Plugin。*
