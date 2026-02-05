# 契约：024-Editor 模块对外 API

## 适用模块

- **实现方**：024-Editor（L4；视口、场景树、属性面板、资源编辑、菜单；无下游）
- **对应规格**：`docs/module-specs/024-editor.md`
- **依赖**：001-Core、003-Application、006-Input、008-RHI、013-Resource、004-Scene、005-Entity、020-Pipeline、018-UI、028-Texture

## 消费者

- 无（L4 消费端）

## 能力列表

### 类型与句柄（跨边界）

本模块不向引擎内其他模块提供跨边界类型；以下为 Editor 内部或向用户/插件暴露的抽象。

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| ViewportHandle | 视口、相机、Gizmo、与 Pipeline/RHI 对接；模型可点击拾取、从资源管理器拖入 | 由 Editor 管理 |
| SceneTreeRef | 场景资源管理器；层级视图、选择、拖拽、复制/粘贴、与 Scene/Entity 对接 | 由 Editor 管理 |
| PropertyPanelRef | 属性面板；与 Object 反射联动、编辑、撤销 | 由 Editor 管理 |
| AssetBrowserRef | 资源浏览器；资源列表、预览、导入、与 Resource 对接 | 由 Editor 管理 |
| MenuRef | 主菜单、工具栏、快捷键、与 Application/Input 对接 | 由 Editor 管理 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 视口 | ViewportHandle、相机、Gizmo、与 Pipeline/RHI 对接；拾取、拖入 |
| 2 | 场景树 | SceneTreeRef、层级、选择、拖拽、与 Scene/Entity 对接 |
| 3 | 属性面板 | PropertyPanelRef、与 Object 反射、编辑、撤销 |
| 4 | 资源浏览器 | AssetBrowserRef、列表、预览、导入、与 Resource 对接 |
| 5 | 菜单 | MenuRef、主菜单、工具栏、快捷键 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在所有依赖模块初始化之后使用。布局与交互约定（左侧场景树、下方资源浏览器、右侧属性面板等）由实现文档化。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 024-Editor 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
