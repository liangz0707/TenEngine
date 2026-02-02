# 018-UI 模块 ABI

- **契约**：[018-ui-public-api.md](./018-ui-public-api.md)（能力与类型描述）
- **本文件**：018-UI 对外 ABI 显式表。
- **参考**：Unity UGUI、UE UMG；画布、控件树、事件、样式。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 画布（Canvas）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 018-UI | TenEngine::ui | ICanvas | 抽象接口 | 创建画布 | TenEngine/ui/Canvas.h | CreateCanvas | `ICanvas* CreateCanvas();` 失败返回 nullptr；与 UICore Layout/Draw/HitTest 对接 |
| 018-UI | TenEngine::ui | ICanvas | 抽象接口 | 添加子节点 | TenEngine/ui/Canvas.h | ICanvas::AddChild | `void AddChild(IWidget* child);` 控件树根下添加 |
| 018-UI | TenEngine::ui | ICanvas | 抽象接口 | 布局与绘制 | TenEngine/ui/Canvas.h | ICanvas::Layout, Draw | `void Layout();` `void Draw(IDrawCommandList* list);` 委托 UICore Measure/Arrange 与 DrawList |
| 018-UI | TenEngine::ui | ICanvas | 抽象接口 | 命中检测 | TenEngine/ui/Canvas.h | ICanvas::HitTest | `HitTestResult HitTest(int x, int y) const;` 委托 UICore HitTest |

### 控件（Widgets）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 018-UI | TenEngine::ui | IWidget | 抽象接口 | 基础控件接口 | TenEngine/ui/Widget.h | IWidget::GetLayoutNode, AddChild, RemoveChild | 与 UICore ILayoutNode 对应；子节点管理 |
| 018-UI | TenEngine::ui | — | 工厂/类型 | 按钮 | TenEngine/ui/Widgets.h | CreateButton | `IWidget* CreateButton(char const* text);` 返回 IWidget*；OnClick 等事件由 SetEventCallback 注册 |
| 018-UI | TenEngine::ui | — | 工厂/类型 | 滑块 | TenEngine/ui/Widgets.h | CreateSlider | `IWidget* CreateSlider(float min, float max, float value);` |
| 018-UI | TenEngine::ui | — | 工厂/类型 | 文本 | TenEngine/ui/Widgets.h | CreateText | `IWidget* CreateText(char const* text);` |
| 018-UI | TenEngine::ui | — | 工厂/类型 | 图片 | TenEngine/ui/Widgets.h | CreateImage | `IWidget* CreateImage(ITextureResource* texture);` |
| 018-UI | TenEngine::ui | — | 工厂/类型 | 列表 | TenEngine/ui/Widgets.h | CreateList | `IWidget* CreateList();` 可滚动列表 |
| 018-UI | TenEngine::ui | — | 工厂/类型 | 容器 | TenEngine/ui/Widgets.h | CreateContainer | `IWidget* CreateContainer();` 布局容器 |
| 018-UI | TenEngine::ui | IWidget | 抽象接口 | 设置样式 | TenEngine/ui/Widget.h | IWidget::SetStyle | `void SetStyle(StyleRef const& style);` 主题、颜色、字体、边距 |

### 事件（Events）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 018-UI | TenEngine::ui | IWidget | 抽象接口 | 注册事件回调 | TenEngine/ui/Events.h | IWidget::SetOnClick, SetOnDrag, SetOnInput | `void SetOnClick(EventCallback cb);` `void SetOnDrag(DragCallback cb);` `void SetOnInput(InputCallback cb);` 与 Input 解耦的抽象事件 |
| 018-UI | TenEngine::ui | — | 枚举/类型 | 事件传播 | TenEngine/ui/Events.h | EventPhase::Bubble, Capture | Bubble/Capture 由实现约定 |

### 样式（Style）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 018-UI | TenEngine::ui | — | struct | 样式引用 | TenEngine/ui/Style.h | StyleRef | 主题、颜色、字体、边距；与 Resource 资源引用对接 |
| 018-UI | TenEngine::ui | — | 自由函数 | 设置主题 | TenEngine/ui/Style.h | SetTheme | `void SetTheme(ICanvas* canvas, StyleRef const& theme);` 与控件或画布绑定 |

*来源：契约能力 Widgets、Canvas、Events、Style；参考 Unity UGUI、UE UMG。*
