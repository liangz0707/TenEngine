# 契约：018-UI 模块对外 API

## 适用模块

- **实现方**：018-UI（L2；控件、画布、事件）
- **对应规格**：`docs/module-specs/018-ui.md`
- **依赖**：017-UICore

## 消费者

- 024-Editor

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| CanvasHandle | 画布句柄；CreateCanvas、AddChild、与 UICore Layout/Draw/HitTest 对接 | 创建后直至显式释放 |
| WidgetTree | 控件树；Button、Slider、Text、Image、List、Container、SetStyle | 与 Canvas 绑定 |
| EventCallback | 事件回调；OnClick、OnDrag、OnInput、Bubble、Capture | 由调用方注册 |
| StyleRef | 主题、颜色/字体/边距、与 Resource 资源引用 | 与控件或画布绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 画布 | CreateCanvas、AddChild；与 UICore Layout/Draw/HitTest 对接 |
| 2 | 控件 | WidgetTree；Button、Slider、Text、Image、List、Container、SetStyle |
| 3 | 事件 | EventCallback、OnClick、OnDrag、OnInput、Bubble、Capture |
| 4 | 样式 | StyleRef、主题、颜色/字体/边距 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 UICore 初始化之后使用。UI 渲染层若由 Pipeline 绘制则 Pipeline 消费 UICore 输出。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 018-UI 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
