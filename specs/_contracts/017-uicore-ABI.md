# 017-UICore 模块 ABI

- **契约**：[017-uicore-public-api.md](./017-uicore-public-api.md)（能力与类型描述）
- **本文件**：017-UICore 对外 ABI 显式表。
- **参考**：Unity Canvas/RectTransform、UE Slate；布局、绘制列表、命中检测、焦点。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 布局（Layout）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 017-UICore | TenEngine::uicore | ILayoutNode | 抽象接口 | 测量与排列 | TenEngine/uicore/Layout.h | ILayoutNode::Measure, Arrange | `void Measure(Size const& available);` `void Arrange(Rect const& finalRect);` 布局规则、与 Application 窗口/DPI 适配 |
| 017-UICore | TenEngine::uicore | ILayoutNode | 抽象接口 | 脏标记 | TenEngine/uicore/Layout.h | ILayoutNode::SetDirty, IsDirty | `void SetDirty();` `bool IsDirty() const;` |
| 017-UICore | TenEngine::uicore | — | 自由函数 | DPI 缩放 | TenEngine/uicore/Layout.h | GetDPIScale | `float GetDPIScale();` 与 Application 窗口/DPI 对接 |
| 017-UICore | TenEngine::uicore | — | struct | 尺寸/矩形 | TenEngine/uicore/Layout.h | Size, Rect | 与布局计算对接 |

### 绘制（Draw）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 017-UICore | TenEngine::uicore | IDrawCommandList | 抽象接口 | 绘制矩形 | TenEngine/uicore/DrawList.h | IDrawCommandList::DrawRect | `void DrawRect(Rect const& rect, Color const& color);` |
| 017-UICore | TenEngine::uicore | IDrawCommandList | 抽象接口 | 绘制纹理 | TenEngine/uicore/DrawList.h | IDrawCommandList::DrawTexture | `void DrawTexture(Rect const& rect, ITextureResource* texture, Rect const* uv);` |
| 017-UICore | TenEngine::uicore | IDrawCommandList | 抽象接口 | 绘制文本 | TenEngine/uicore/DrawList.h | IDrawCommandList::DrawText | `void DrawText(Rect const& rect, char const* text, IFont* font, Color const& color);` |
| 017-UICore | TenEngine::uicore | IDrawCommandList | 抽象接口 | 提交到管线 | TenEngine/uicore/DrawList.h | IDrawCommandList::SubmitToPipeline | `void SubmitToPipeline(IRenderPipeline* pipeline);` 与 Pipeline 2D/Overlay 层对接 |
| 017-UICore | TenEngine::uicore | — | 自由函数/工厂 | 创建绘制列表 | TenEngine/uicore/DrawList.h | CreateDrawCommandList | `IDrawCommandList* CreateDrawCommandList();` 单帧或单次提交周期；调用方释放或由框架回收 |

### 命中检测与焦点（HitTest / Focus）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 017-UICore | TenEngine::uicore | — | 自由函数/接口 | 命中检测 | TenEngine/uicore/HitTest.h | HitTest | `HitTestResult HitTest(ILayoutNode* root, int x, int y);` 与 Input 事件路由对接 |
| 017-UICore | TenEngine::uicore | — | struct | 命中结果 | TenEngine/uicore/HitTest.h | HitTestResult | node、localPoint 等；单次查询有效 |
| 017-UICore | TenEngine::uicore | IFocusChain | 抽象接口 | 获取/设置焦点 | TenEngine/uicore/Focus.h | IFocusChain::GetFocus, SetFocus | `ILayoutNode* GetFocus() const;` `void SetFocus(ILayoutNode* node);` |
| 017-UICore | TenEngine::uicore | IFocusChain | 抽象接口 | 路由事件 | TenEngine/uicore/Focus.h | IFocusChain::RouteEvent | `bool RouteEvent(InputEvent const& ev);` 与 Input 事件语义一致 |
| 017-UICore | TenEngine::uicore | — | 自由函数/单例 | 获取焦点链 | TenEngine/uicore/Focus.h | GetFocusChain | `IFocusChain* GetFocusChain();` 由 UICore 管理；调用方不拥有指针 |

### 字体（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 017-UICore | TenEngine::uicore | IFont | 抽象接口 | 加载字体 | TenEngine/uicore/Font.h | LoadFont | `IFont* LoadFont(char const* path);` 与 Resource 对接（可选）；失败返回 nullptr |
| 017-UICore | TenEngine::uicore | IFont | 抽象接口 | 字形与图集 | TenEngine/uicore/Font.h | IFont::GetGlyph, GetAtlas | `GlyphInfo GetGlyph(uint32_t codePoint) const;` `ITextureResource* GetAtlas() const;` |

*来源：契约能力 Layout、Draw、HitTest、Font；参考 Unity Canvas、UE Slate。*
