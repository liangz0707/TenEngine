# 契约：024-Editor 模块对外 API

## 适用模块

- **实现方**：**024-Editor**（视口、场景树、属性面板、资源编辑与菜单）
- **对应规格**：`docs/module-specs/024-editor.md`
- **依赖**：Core、Application、Input、RHI、Resource、Scene、Entity、Pipeline、UI（见 `000-module-dependency-map.md`）

## 消费者（T0 下游）

- **无**（Editor 为 L4 消费端；仅消费各模块 API，不向引擎内其他模块提供 API）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

本模块**不向引擎内其他模块提供**跨边界类型；以下为 Editor 内部或向**用户/插件**暴露的抽象（可选文档化）：

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ViewportHandle | 视口窗口、相机、Gizmo、与 Pipeline/RHI 对接 | 由 Editor 管理 |
| SceneTreeRef | 层级视图、选择、拖拽、复制/粘贴、与 Scene/Entity 对接 | 由 Editor 管理 |
| PropertyPanelRef | 属性编辑、与 Object 反射联动、撤销 | 由 Editor 管理 |
| AssetBrowserRef | 资源列表、预览、导入、与 Resource 对接 | 由 Editor 管理 |
| MenuRef | 主菜单、工具栏、快捷键、与 Application/Input 对接 | 由 Editor 管理 |

若未来支持**编辑器插件**或**脚本**调用 Editor 能力，可在本契约中补充对外句柄与能力列表。

## 能力列表（提供方保证）

1. **Viewport**：CreateViewport、SetCamera、Gizmo、Render；与 Pipeline/RHI 对接。
2. **SceneTree**：ShowHierarchy、Select、DragDrop、CopyPaste；与 Scene/Entity 对接。
3. **PropertyPanel**：BindObject、EditProperty、UndoRedo；与 Object 对接。
4. **AssetBrowser**：ListAssets、Preview、Import；与 Resource 对接。
5. **Menu**：MainMenu、Toolbar、Shortcut；与 Application/Input 对接。

## 调用顺序与约束

- 须在所有依赖模块初始化之后使用；视口与 Pipeline/RHI、场景树与 Scene/Entity、属性与 Object 协同一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：024-Editor 对应本契约；无下游；与 docs/module-specs/024-editor.md 一致 |
