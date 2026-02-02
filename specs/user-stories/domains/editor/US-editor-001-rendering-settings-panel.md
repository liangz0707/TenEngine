# US-editor-001：编辑器内配置渲染设置并保存

- **标题**：用户可在编辑器内配置各种渲染设置；顶部有按钮选项，点击后右侧显示渲染配置面板；可配置延迟/前向/全 Compute 渲染、灯光/阴影/太阳光/IBL/DOF/抗锯齿等开关；可保存配置。
- **编号**：US-editor-001

---

## 1. 角色/触发

- **角色**：使用编辑器的开发者
- **触发**：在编辑器界面顶部点击「渲染设置」等按钮后，右侧弹出/显示**渲染配置面板**；用户在面板中修改各项渲染选项，并可保存为配置（项目或预设）。

---

## 2. 端到端流程

1. 用户在编辑器**顶部**点击与渲染相关的按钮（如「渲染设置」「Rendering」）。
2. **Editor** 在**右侧**显示**渲染配置面板**（若已有面板则聚焦/展开）。
3. 面板内提供以下可配置项（示例，可按实现扩展）：
   - **渲染路径**：延迟渲染、前向渲染、全 Compute 渲染（三选一或兼容多选）。
   - **灯光与光照**：开关灯光、开关阴影、开关太阳光、开关 IBL（环境光）。
   - **后处理**：开关 DOF（景深）屏幕后处理。
   - **抗锯齿**：开关或选择多种抗锯齿（如 MSAA、TAA、FXAA、关闭）。
4. 用户修改选项后，**实时或按「应用」** 将配置下发至当前视口使用的 **Pipeline/Effects**，视口预览立即反映变化。
5. 用户点击「保存」时，**Editor** 将当前配置持久化（项目设置文件或预设资源）；下次打开项目或加载该预设时可恢复。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 024-Editor | 顶部按钮、右侧渲染配置面板 UI；RenderingConfig 数据结构；保存/加载配置到文件或预设 |
| 020-Pipeline | 根据 RenderingConfig 选择渲染路径（延迟/前向/全 Compute）并应用灯光/阴影/太阳/IBL 等开关 |
| 021-Effects | 根据配置开关 DOF、抗锯齿等后处理 |
| 018-UI | 面板布局、控件（按钮、勾选框、下拉框）由 Editor 使用 UI 模块绘制 |

---

## 4. 每模块职责与 I/O

### 024-Editor

- **职责**：提供「渲染设置」入口（顶部按钮）；管理右侧**渲染配置面板**的显示与内容；持有 **RenderingConfig** 结构，与面板控件双向绑定；用户点击保存时，将 RenderingConfig 序列化到项目设置或预设文件；加载时从文件反序列化并填充面板与 Pipeline。
- **输入**：用户点击顶部按钮、面板内控件变更、保存/加载请求；当前 Pipeline 的 getRenderingConfig() 用于初始化或同步面板。
- **输出**：`IRenderingSettingsPanel`（show/hide、getConfig/setConfig、saveConfig/loadConfig）；`RenderingConfig` 结构体；面板显示时从 Pipeline 拉取当前配置，用户修改后通过 Pipeline::setRenderingConfig 下发。

### 020-Pipeline

- **职责**：提供 **setRenderingConfig** / **getRenderingConfig**，供 Editor 读取当前配置与下发新配置；根据配置切换渲染路径（延迟/前向/全 Compute）、灯光/阴影/太阳/IBL 开关；与 021-Effects 协同处理 DOF、抗锯齿等（或由 Pipeline 统一转发配置）。
- **输入**：Editor 调用的 setRenderingConfig(RenderingConfig const&)。
- **输出**：IRenderPipeline::setRenderingConfig、getRenderingConfig；下一帧渲染时按新配置生效。

### 021-Effects

- **职责**：根据 RenderingConfig 中的后处理与抗锯齿开关，启用/禁用 DOF、各类抗锯齿（MSAA/TAA/FXAA 等）；配置由 Pipeline 下发或由 Editor 通过 Pipeline 统一设置。
- **输入**：RenderingConfig（或 Pipeline 转发的 effects 相关字段）。
- **输出**：效果开关与 Pipeline 配置一致；可选 IEffectsConfig 或复用 RenderingConfig 的子集。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 024-Editor

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 024-Editor | TenEngine::editor | IRenderingSettingsPanel | 抽象接口 | 渲染配置面板 | TenEngine/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel::show, hide | void show(); void hide(); 点击顶部按钮后显示/隐藏右侧面板 |
| 024-Editor | TenEngine::editor | IRenderingSettingsPanel | 抽象接口 | 获取/设置配置 | TenEngine/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel::getConfig, setConfig | RenderingConfig getConfig() const; void setConfig(RenderingConfig const&); 与面板控件双向绑定 |
| 024-Editor | TenEngine::editor | IRenderingSettingsPanel | 抽象接口 | 保存/加载配置 | TenEngine/editor/RenderingSettingsPanel.h | IRenderingSettingsPanel::saveConfig, loadConfig | bool saveConfig(char const* path); bool loadConfig(char const* path); 持久化到项目或预设文件 |
| 024-Editor | TenEngine::editor | — | struct | 渲染配置数据 | TenEngine/editor/RenderingConfig.h | RenderingConfig | 见下表字段；与 Pipeline/Effects 共用或由 Editor 定义后 Pipeline 引用 |
| 024-Editor | TenEngine::editor | IEditor | 抽象接口 | 获取渲染设置面板 | TenEngine/editor/Editor.h | IEditor::getRenderingSettingsPanel | IRenderingSettingsPanel* getRenderingSettingsPanel(); 用于顶部按钮点击后显示面板 |

**RenderingConfig 字段（示例）**：

| 字段 | 类型 | 说明 |
|------|------|------|
| renderingPath | enum | Deferred / Forward / FullCompute |
| lightsEnabled | bool | 开关灯光 |
| shadowsEnabled | bool | 开关阴影 |
| sunLightEnabled | bool | 开关太阳光 |
| iblEnabled | bool | 开关 IBL 环境光 |
| dofEnabled | bool | 开关 DOF 景深后处理 |
| antiAliasing | enum | None / MSAA / TAA / FXAA 等 |

（具体枚举名与可选字段以实现为准；Editor 与 Pipeline 共用的定义可放在 020-Pipeline 或公共头文件中，由 Editor 引用。）

### 020-Pipeline

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 020-Pipeline | TenEngine::pipeline | IRenderPipeline | 抽象接口 | 设置/获取渲染配置 | TenEngine/pipeline/RenderPipeline.h | IRenderPipeline::setRenderingConfig, getRenderingConfig | void setRenderingConfig(RenderingConfig const&); RenderingConfig getRenderingConfig() const; Editor 下发配置，下一帧生效；Pipeline 负责路径与灯光/阴影/太阳/IBL；DOF/AA 可转发 Effects |
| 020-Pipeline | TenEngine::pipeline | — | struct | 渲染配置（与 Editor 共用） | TenEngine/pipeline/RenderingConfig.h | RenderingConfig | 渲染路径、灯光/阴影/太阳/IBL/DOF/抗锯齿等；Editor 与 Pipeline 共用的定义可放本模块或 024 引用 |

### 021-Effects

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 021-Effects | TenEngine::effects | — | struct | 与 RenderingConfig 一致或子集 | TenEngine/effects/EffectsConfig.h | 见 Pipeline RenderingConfig | DOF、抗锯齿等由 Pipeline 的 RenderingConfig 转发或本模块提供 applyConfig(RenderingConfig const&) |

（若 RenderingConfig 由 020-Pipeline 定义并包含 effects 相关字段，021-Effects 仅需按 Pipeline 下发的配置启用/禁用效果，可不单独定义新类型。）

---

## 6. 参考（可选）

- **Unity**：Edit → Project Settings → Graphics / URP HDRP 设置；Window → Rendering 等；预设保存与加载。
- **Unreal**：Project Settings → Engine - Rendering；Editor 偏好设置中渲染相关项；Config 文件保存。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/024-editor-ABI.md`、`020-pipeline-ABI.md`、`021-effects-ABI.md`（若需）。*
