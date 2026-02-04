# 024-Editor 模块 ABI

- **契约**：[024-editor-public-api.md](./024-editor-public-api.md)（能力与类型描述）
- **本文件**：024-Editor 对外 ABI 显式表。
- **布局约定**：**左侧**场景资源管理器、**下方**资源浏览器、**右侧**属性面板、**中间**渲染窗口；渲染窗口中模型可**点击拾取**，可从资源管理器**拖入**；右侧属性面板可**显示各种 Component 的属性**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | te::editor | IEditor | 抽象接口 | 编辑器主入口 | te/editor/Editor.h | IEditor::Run | `void Run(EditorContext const& ctx);` 进入编辑器主循环（视口、场景树、属性、资源浏览器），直至用户关闭；仅在 RunMode::Editor 时由 Application 调用 |
| 024-Editor | te::editor | — | struct | 编辑器上下文 | te/editor/Editor.h | EditorContext | 窗口句柄、可选 RHI/Scene/Resource 等句柄；由 Application 或 SubsystemRegistry 在启动 Editor 时填充 |
| 024-Editor | te::editor | — | 自由函数/工厂 | 创建编辑器实例 | te/editor/Editor.h | CreateEditor | `IEditor* CreateEditor(EditorContext const& ctx);` 或通过 SubsystemRegistry::GetSubsystem\<IEditor\>() 获取 |
| 024-Editor | te::editor | IEditor | 抽象接口 | **左侧**场景资源管理器 | te/editor/SceneView.h | IEditor::GetSceneView | `ISceneView* GetSceneView();` 返回左侧场景资源管理器接口（场景层级、Entity 树）；与 Scene/Entity 对接 |
| 024-Editor | te::editor | IEditor | 抽象接口 | **下方**资源浏览器 | te/editor/ResourceView.h | IEditor::GetResourceView | `IResourceView* GetResourceView();` 返回下方资源浏览器接口；与 Resource 对接；支持从资源管理器拖入到渲染窗口或场景 |
| 024-Editor | te::editor | IEditor | 抽象接口 | **右侧**属性面板 | te/editor/PropertyPanel.h | IEditor::GetPropertyPanel | `IPropertyPanel* GetPropertyPanel();` 返回右侧属性面板接口；可显示各种 Component 的属性；与 Object 反射联动 |
| 024-Editor | te::editor | IEditor | 抽象接口 | **中间**渲染窗口 | te/editor/Viewport.h | IEditor::GetRenderViewport | `IViewport* GetRenderViewport();` 返回中间渲染窗口接口；可点击拾取、从资源管理器拖入；相机、Gizmo；与 Pipeline/RHI 对接 |
| 024-Editor | te::editor | IEditor | 抽象接口 | 获取渲染设置面板 | te/editor/Editor.h | IEditor::GetRenderingSettingsPanel | `IRenderingSettingsPanel* GetRenderingSettingsPanel();` 顶部按钮点击后显示右侧渲染配置面板 |
| 024-Editor | te::editor | IRenderingSettingsPanel | 抽象接口 | 渲染配置面板 | te/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel::Show, Hide | `void Show();` `void Hide();` 显示/隐藏右侧渲染配置面板 |
| 024-Editor | te::editor | IRenderingSettingsPanel | 抽象接口 | 获取/设置配置 | te/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel::GetConfig, SetConfig | `RenderingConfig GetConfig() const;` `void SetConfig(RenderingConfig const& config);` 与面板控件双向绑定 |
| 024-Editor | te::editor | IRenderingSettingsPanel | 抽象接口 | 保存/加载配置 | te/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel::SaveConfig, LoadConfig | `bool SaveConfig(char const* path);` `bool LoadConfig(char const* path);` 持久化到项目或预设文件 |
| 024-Editor | te::editor | — | struct | 渲染配置数据 | te/editor/RenderingConfig.h | RenderingConfig | 渲染路径、灯光/阴影/太阳/IBL/DOF/抗锯齿等；与 Pipeline 共用定义 |
| 024-Editor | te::editor | IViewport | 抽象接口 | 视口拾取 | te/editor/Viewport.h | IViewport::PickInViewport | `IEntity* PickInViewport(int x, int y) const;` 点击拾取模型/Entity；与 Pipeline/射线对接 |
| 024-Editor | te::editor | IViewport | 抽象接口 | 从资源管理器拖入 | te/editor/Viewport.h | IViewport::DropFromResourceManager | `void DropFromResourceManager(ResourceId resource_id, int x, int y);` 将资源拖入场景或视口放置 |
| 024-Editor | te::editor | IPropertyPanel | 抽象接口 | 撤销/重做 | te/editor/PropertyPanel.h | IPropertyPanel::Undo, Redo, CanUndo, CanRedo | `void Undo();` `void Redo();` `bool CanUndo() const;` `bool CanRedo() const;` 与 Object 属性编辑联动 |

*来源：用户故事 US-lifecycle-002、US-editor-001；契约能力：PickInViewport、DropFromResourceManager、UndoRedo。*
