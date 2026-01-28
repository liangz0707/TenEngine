# 契约：018-UI 模块对外 API

## 适用模块

- **实现方**：**018-UI**（控件、画布与事件；依赖 UICore）
- **对应规格**：`docs/module-specs/018-ui.md`
- **依赖**：017-UICore（017-uicore-public-api）

## 消费者（T0 下游）

- 024-Editor（窗口、面板、菜单、控件；UI 渲染层若由 Pipeline 绘制则 Pipeline 消费 UICore 输出）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| CanvasHandle | 画布句柄；CreateCanvas、AddChild、与 UICore Layout/Draw/HitTest 对接 | 创建后直至显式释放 |
| WidgetTree | 控件树；Button、Slider、Text、Image、List、Container、SetStyle | 与 Canvas 绑定 |
| EventCallback | 事件回调；OnClick、OnDrag、OnInput、Bubble、Capture | 由调用方注册 |
| StyleRef | 主题、颜色/字体/边距、与 Resource 资源引用 | 与控件或画布绑定 |

下游仅通过上述类型与句柄访问；与 UICore 的布局/绘制/命中一一对应，与 Resource 纹理/图集/字体通过 UICore 或直接对接。

## 能力列表（提供方保证）

1. **Widgets**：Button、Slider、Text、Image、List、Container、SetStyle；基础与复合控件、样式。
2. **Canvas**：CreateCanvas、AddChild；与 UICore Layout/Draw/HitTest 对接。
3. **Events**：OnClick、OnDrag、OnInput、Bubble、Capture；与 Input 解耦的抽象事件。
4. **Style**：Theme、Color、Font、Margin、ResourceRef。

## 调用顺序与约束

- 须在 UICore 初始化之后使用；画布与控件树与 UICore 布局/绘制/命中语义一致。
- Editor 消费本 API 时，视口/面板/菜单与 Application/Input 协同。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：018-UI 对应本契约；与 docs/module-specs/018-ui.md 一致 |
