# 契约：017-UICore 模块对外 API

## 适用模块

- **实现方**：**017-UICore**（UI 布局、绘制与输入路由）
- **对应规格**：`docs/module-specs/017-ui-core.md`
- **依赖**：001-Core（001-core-public-api）、003-Application（003-application-public-api）、006-Input（006-input-public-api）

## 消费者（T0 下游）

- 018-UI（控件与画布、布局、绘制、事件）
- 024-Editor（编辑器 UI、视口 Overlay）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| LayoutNode | 布局树节点；Measure、Arrange、SetDirty、DPIScale | 由控件树管理 |
| DrawCommandList | 绘制列表/命令；DrawRect、DrawTexture、DrawText、SubmitToPipeline | 单帧或单次提交周期 |
| HitTestResult | 命中检测结果；与 Input 事件路由对接 | 单次查询 |
| FocusChain | 焦点链；GetFocus、SetFocus、RouteEvent | 由 UICore 管理 |
| FontHandle | 字体句柄；LoadFont、GetGlyph、Atlas、与 Resource 对接（可选） | 创建后直至显式释放 |

下游仅通过上述类型与句柄访问；绘制命令由 Pipeline 的 2D 或 Overlay 层消费，本模块不直接持有 GPU 资源。**ABI 显式表**：[017-uicore-ABI.md](./017-uicore-ABI.md)。

## 能力列表（提供方保证）

1. **Layout**：ILayoutNode::Measure、Arrange、SetDirty、IsDirty；GetDPIScale；Size、Rect；布局规则、脏标记、与 Application 窗口/DPI 适配。
2. **Draw**：IDrawCommandList::DrawRect、DrawTexture、DrawText、SubmitToPipeline；CreateDrawCommandList；与渲染后端接口对接。
3. **HitTest**：HitTest、HitTestResult；IFocusChain::GetFocus、SetFocus、RouteEvent；GetFocusChain；命中检测、焦点与 Input 事件路由。
4. **Font**：LoadFont、IFont::GetGlyph、GetAtlas；与 Resource 对接（可选）。

## 调用顺序与约束

- 须在 Core、Application、Input 初始化之后使用；与 Application 窗口/DPI、Input 事件语义一致。
- 绘制命令提交时机与 Pipeline 的 2D/Overlay 消费约定须一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 按 017-UICore 模块规格与依赖表新增契约；类型与能力与 docs/module-specs/017-ui-core.md 一致 |
| 2026-01-28 | 根据 017-uicore-ABI 反向更新：ILayoutNode、IDrawCommandList、HitTest、IFocusChain、LoadFont、CreateDrawCommandList；能力与类型与 ABI 表一致 |
