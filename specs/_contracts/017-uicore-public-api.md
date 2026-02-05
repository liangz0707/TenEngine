# 契约：017-UICore 模块对外 API

## 适用模块

- **实现方**：017-UICore（L2；UI 布局、绘制、输入路由）
- **对应规格**：`docs/module-specs/017-ui-core.md`
- **依赖**：001-Core、003-Application、006-Input

## 消费者

- 018-UI、024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| LayoutNode | 布局树节点；Measure、Arrange、SetDirty、DPIScale | 由控件树管理 |
| DrawCommandList | 绘制列表/命令；DrawRect、DrawTexture、DrawText、SubmitToPipeline | 单帧或单次提交周期 |
| HitTestResult | 命中检测结果；与 Input 事件路由对接 | 单次查询 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 布局 | LayoutNode、Measure、Arrange、SetDirty、DPIScale |
| 2 | 绘制 | DrawCommandList、DrawRect、DrawTexture、DrawText、SubmitToPipeline |
| 3 | 输入路由 | HitTestResult、与 Input 事件对接；焦点管理 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Application、Input 初始化之后使用。与 018-UI 控件树、024-Editor 视口 Overlay 对接须明确。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 017-UICore 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
