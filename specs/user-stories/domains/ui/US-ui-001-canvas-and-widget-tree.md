# US-ui-001：画布与控件树（Canvas、控件层级、布局、与 UICore 对接）

- **标题**：应用可创建**画布**（Canvas）、在画布下构建**控件树**（按钮、文本、图片等）、设置**布局**（锚点、相对布局）；UI 由 UICore 绘制、与 Pipeline 对接渲染、与 Input 对接点击。
- **编号**：US-ui-001

---

## 1. 角色/触发

- **角色**：游戏逻辑、Editor
- **触发**：需要显示 **UI**（HUD、菜单、对话框）；UI 由**画布**与**控件树**组成；布局可锚点/相对；点击等事件由 Input 与 UICore 焦点协同。

---

## 2. 端到端流程

1. 调用方创建 **Canvas**（或从场景/资源加载）；在 Canvas 下创建**控件**（Button、Text、Image 等），形成**控件树**；设置控件**布局**（rect、anchor、pivot）。
2. **UI** 模块与 **UICore** 对接：UICore 负责**布局计算**、**绘制列表**、**命中检测**；每帧 Pipeline 执行 UI Pass 时，UICore 产出绘制命令或几何，由 Pipeline 提交到 RHI。
3. **Input** 事件（点击、悬停）经 UICore **命中检测** 落到具体控件；控件可订阅 **onClick**、**onHover** 等；焦点由 UICore 管理。
4. Editor 的 UI 可复用同一套 UICore/UI 模块，或使用独立实现；由项目约定。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 018-UI | Canvas、控件树、Button/Text/Image、布局（rect、anchor）、与 UICore 对接 |
| 017-UICore | 布局、绘制列表、命中检测、焦点；与 Pipeline/Input 对接 |
| 020-Pipeline | UI Pass、渲染 UICore 产出；与 RHI 对接 |
| 006-Input | 鼠标/触摸事件；UICore 命中检测后驱动控件事件 |

---

## 4. 每模块职责与 I/O

### 018-UI

- **职责**：提供 **Canvas**、**控件类型**（Button、Text、Image 等）、**控件树**（父子、层级）、**布局属性**（rect、anchor、pivot）；与 UICore 对接布局与绘制；与 Input 协同焦点与事件。
- **输入**：控件创建/销毁、布局变更、事件订阅。
- **输出**：画布与控件树、布局结果；供 UICore 绘制与命中检测。

---

## 5. 派生 ABI（与契约对齐）

- **018-ui-ABI**：Canvas、Widget、Button、Text、Image、布局 API；与 UICore 对接。详见 `specs/_contracts/018-ui-ABI.md`。

---

## 6. 验收要点

- 可创建画布与控件树、设置布局；UI 由 UICore 绘制、经 Pipeline 渲染。
- 点击等输入经命中检测落到控件，可订阅控件事件。
