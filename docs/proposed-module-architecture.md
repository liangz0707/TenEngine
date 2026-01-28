# TenEngine 整合式模块划分与依赖设计（提案）

本提案在现有 TenEngine 规格基础上，整合 **Unreal Engine** 与 **Unity** 的模块划分与依赖思路，给出更清晰的分层与依赖关系，便于并行开发、测试与扩展。设计遵循 Constitution 的模块化、现代图形 API、数据驱动与版本化原则。

**说明**：本文为备选/历史提案。**当前 T0 架构**为 27 模块（001–027），契约见 `specs/_contracts/`：`NNN-modulename-public-api.md`（如 `001-core-public-api.md`、`008-rhi-public-api.md`）、边界契约为 `pipeline-to-rci.md`；依赖表见 `000-module-dependency-map.md`。

---

## 一、设计原则（来自 UE / Unity）

| 原则 | 来源 | 在 TenEngine 的体现 |
|------|------|---------------------|
| **单一根模块，无业务依赖** | UE Core | Core 仅提供内存/线程/平台/ECS/序列化，不依赖任何业务模块。 |
| **图形 API 与管线逻辑分离** | UE RHI vs Renderer | RHI（RCI）只做 GPU 抽象；管线类型与 Pass 协议在 RenderCore/PipelineCore。 |
| **渲染管线“核心 + 实现”分层** | Unity SRP Core + URP/HDRP | 引入 PipelineCore（共享类型、命令缓冲协议、Pass 图协议），Pipeline 实现场景→DrawCall→命令缓冲。 |
| **渲染资源与 Shader 类型共享层** | UE RenderCore | 引入 RenderCore：Shader 参数结构、渲染资源描述、Pass 参数协议，无场景逻辑。 |
| **编辑器仅依赖运行时，永不反向** | UE UnrealEd | Editor 依赖 Core、RHI、Resource、Pipeline（或仅 RHI+Resource 做预览），运行时模块不依赖 Editor。 |
| **资源系统与渲染/管线解耦** | Unity Addressables | Resource 只依赖 Core；管线通过“资源句柄/描述”消费资源，不直接依赖 Resource 实现。 |
| **可插拔子系统（可选）** | Unity Subsystems | Core 内平台抽象可扩展为“子系统”注册（输入、显示等），便于可选功能与多后端。 |

---

## 二、提议的模块列表与层级

### 2.1 层级定义

- **L0 基础**：无任何业务依赖，所有模块最终依赖于此。
- **L1 平台/抽象**：依赖 L0，提供跨平台或跨 API 的抽象。
- **L2 核心能力**：依赖 L0/L1，提供渲染核心类型、Shader、资源等，无完整管线/场景逻辑。
- **L3 管线与场景**：依赖 L2，产出可执行命令缓冲或完整场景渲染。
- **L4 工具/编辑器**：依赖 L0–L3，仅用于编辑/工具，不参与纯运行时依赖链。

### 2.2 模块列表（提议）

| 层级 | 编号 | 模块名 | 职责摘要 | 对应 UE/Unity 概念 |
|------|------|--------|----------|---------------------|
| **L0** | 001 | **Core** | 内存、线程、平台（文件/时间/环境）、日志、ECS、序列化；动态库交付 | UE: Core + CoreUObject；Unity: Engine Core |
| **L1** | 002 | **RHI** | 图形 API 抽象（Vulkan/D3D12/Metal）；命令列表、资源、PSO；多后端 DLL/源码 | UE: RHI + RHICore；Unity: 底层图形封装 |
| **L2** | 003 | **RenderCore** | Shader 参数结构、渲染资源描述、Pass 参数协议、Uniform Buffer 约定；无场景 | UE: RenderCore；Unity: SRP Core 部分 |
| **L2** | 004 | **Shader** | Shader 读取、编译、预编译、变体；库/源码/工具形态 | UE: RenderCore 内 + 材质；Unity: Shader Graph 依赖层 |
| **L2** | 005 | **Resource** | 资源导入、同步/异步加载卸载、加载状态与限制；与渲染解耦 | UE: Engine 资源系统；Unity: Addressables + 导入 |
| **L3** | 006 | **PipelineCore** | 管线共享类型、命令缓冲格式、Pass 图协议（RDG 风格）；无场景 | Unity: SRP Core（管线协议部分） |
| **L3** | 007 | **Pipeline** | 场景收集→剔除→DrawCall→命令缓冲；消费 PipelineCore 协议，向 RHI 提交 | UE: Renderer（RDG 使用方）；Unity: URP/HDRP 实现 |
| **L4** | 008 | **Editor** | 面板、布局、文件管理、渲染视口、属性面板、场景树 | UE: UnrealEd + Slate；Unity: Editor + uGUI |
| **L4** | 009 | **Tools** | 第三方集成、命令行/批处理工具 | 各引擎 Plugins/Programs |

说明：当前 TenEngine 已有 001–006 及 006-thirdparty；本提案将 **RCI** 保留为对外名称、概念上对应 **RHI**；**RenderCore**、**PipelineCore** 为新增概念层，可与现有 002/006 通过“子模块”或“契约拆分”逐步对齐。

---

## 三、依赖关系（提议）

### 3.1 依赖表（下游 → 上游）

| 模块 | 直接依赖 | 契约/接口 |
|------|----------|-----------|
| 001-Core | — | core-public-api |
| 002-RHI | 001-Core | rhi-public-api（即原 rci-public-api） |
| 003-RenderCore | 001-Core, 002-RHI | render-core-api |
| 004-Shader | 001-Core, 002-RHI, 003-RenderCore | shader-public-api |
| 005-Resource | 001-Core | resource-public-api |
| 006-PipelineCore | 002-RHI, 003-RenderCore | pipeline-core-api, pipeline-to-rhi |
| 007-Pipeline | 001-Core, 006-PipelineCore, 003-RenderCore | pipeline-public-api |
| 008-Editor | 001-Core, 002-RHI, 005-Resource, 007-Pipeline（可选） | editor-public-api |
| 009-Tools | 按需 | — |

### 3.2 依赖规则（强制）

1. **L0 → 无依赖**：Core 不依赖任何其他业务模块。
2. **L1 仅依赖 L0**：RHI 只依赖 Core（平台、内存等）。
3. **L2 依赖 L0/L1，不依赖 L3/L4**：RenderCore、Shader、Resource 不依赖 Pipeline 或 Editor。
4. **L3 依赖 L0–L2，不依赖 L4**：PipelineCore、Pipeline 不依赖 Editor。
5. **L4 可依赖 L0–L3**：Editor、Tools 可依赖所有运行时模块，反之不可。
6. **渲染链单向**：RHI ← RenderCore ← PipelineCore ← Pipeline；Pipeline 产出命令缓冲，由 RHI 执行。
7. **Resource 与渲染解耦**：Resource 不依赖 RHI/RenderCore/Pipeline；管线通过“资源句柄/描述”使用资源（由 Core 或契约约定）。

### 3.3 依赖图（ASCII）

```
                                    ┌─────────────────────────┐
                                    │ 008-Editor (L4)         │
                                    │ 009-Tools (L4)          │
                                    └───────────┬─────────────┘
                                                │
    ┌─────────────┐     ┌─────────────┐         │         ┌─────────────┐
    │ 005-Resource│     │ 007-Pipeline│         │         │ 004-Shader  │
    │ (L2)        │     │ (L3)        │◄────────┘         │ (L2)        │
    └──────┬──────┘     └──────┬──────┘                     └──────┬──────┘
           │                   │                                   │
           │                   │  ┌─────────────────────┐          │
           │                   └─►│ 006-PipelineCore   │◄─────────┘
           │                      │ (L3, 协议层)         │
           │                      └──────────┬──────────┘
           │                                 │
           │                                 ▼
           │                      ┌─────────────────────┐
           │                      │ 003-RenderCore (L2) │
           │                      │ (Shader/Pass 类型)   │
           │                      └──────────┬──────────┘
           │                                 │
           │                                 ▼
           │                      ┌─────────────────────┐
           └────────────────────►│ 002-RHI (L1)        │
                                  │ (图形 API 抽象)     │
                                  └──────────┬──────────┘
                                             │
                                             ▼
                                  ┌─────────────────────┐
                                  │ 001-Core (L0)       │
                                  │ (内存/线程/ECS/     │
                                  │  平台/序列化)        │
                                  └─────────────────────┘
```

---

## 四、与当前 TenEngine 的对应关系

| 当前 spec | 提议模块 | 说明 |
|-----------|----------|------|
| 001-engine-core-module | 001-Core | 一致，可补充内部子域（Foundation / Object） |
| 002-rendering-rci-interface | 002-RHI | 一致，RCI 即 RHI 对外命名 |
| 003-editor-system | 008-Editor | 编号调整，职责一致 |
| 004-resource-system | 005-Resource | 编号调整，职责一致；强调与渲染解耦 |
| 005-shader-system | 004-Shader | 编号调整，增加对 RenderCore 的依赖（类型/描述） |
| 006-render-pipeline-system | 006-PipelineCore + 007-Pipeline | 拆分为“管线协议层”与“管线实现”；当前 006 可先对应 007，契约中补充 PipelineCore |
| 006-thirdparty-integration-tool | 009-Tools | 编号调整，职责一致 |
| — | 003-RenderCore | 新增；可从 002/006 中抽出“共享类型与协议” |

---

## 五、契约与接口建议

| 契约文件 | 提供方 | 主要内容 |
|----------|--------|----------|
| core-public-api | 001-Core | 内存、线程、ECS、平台、序列化（现有） |
| rhi-public-api | 002-RHI | 命令列表、资源、PSO、提交接口（即原 rci-public-api） |
| render-core-api | 003-RenderCore | Shader 参数结构、渲染资源描述、Pass 参数协议、UB 约定 |
| pipeline-core-api | 006-PipelineCore | 命令缓冲格式、Pass 图协议、与 RHI 的提交约定（即 pipeline-to-rci 扩展） |
| pipeline-to-rhi | 006-PipelineCore / 002-RHI | Pipeline 产出命令缓冲，RHI 消费（现有 pipeline-to-rci 可重命名） |
| resource-public-api | 005-Resource | 导入、加载/卸载、状态、句柄（可选，与 Core 配合） |
| shader-public-api | 004-Shader | 编译、变体、产物与 RenderCore/RHI 的对接 |
| editor-public-api | 008-Editor | 面板、视口、场景树 等对外扩展点（可选） |

---

## 六、实施建议（分阶段）

1. **阶段 1（当前可做）**  
   - 保持现有 001–006 编号与 spec。  
   - 在契约与文档中明确 **RenderCore**、**PipelineCore** 为“逻辑层”：002 的公开类型与 006 的“命令缓冲/Pass 协议”视为 RenderCore/PipelineCore 的契约内容。  
   - 依赖图按本节“提议依赖”在文档中标注为目标状态，现有 003/004/005/006 对 Core/RCI 的依赖保持不变。

2. **阶段 2（可选拆分）**  
   - 将 002（RCI）中与“Shader 参数/渲染资源描述”相关的内容抽到独立 **003-RenderCore** 模块（或 002 子目录 + 独立契约 render-core-api）。  
   - 将 006 中“命令缓冲格式、Pass 图协议”单独成约 **pipeline-core-api**，006 实现变为“Pipeline 实现”，依赖 PipelineCore 契约。

3. **阶段 3（可选编号迁移）**  
   - 若希望目录与编号完全对齐提议：新增 specs/003-render-core、006-pipeline-core、007-pipeline，并将现有 003/004/005 重编号为 008/005/004 等；或保留现有编号，仅在文档和契约中使用“L0–L4 + 模块名”表述。

---

## 七、小结

- **L0**：Core（唯一根）。  
- **L1**：RHI（图形 API 抽象）。  
- **L2**：RenderCore（渲染类型/协议）、Shader、Resource（与渲染解耦）。  
- **L3**：PipelineCore（管线协议）、Pipeline（场景→命令缓冲）。  
- **L4**：Editor、Tools。  

依赖规则：单向、无环；Editor 只依赖运行时；Resource 不依赖渲染；渲染链为 RHI ← RenderCore ← PipelineCore ← Pipeline。  

本提案可与现有 `specs/_contracts/` 及 `000-module-dependency-map.md` 并行使用：契约与依赖图逐步向本节“提议依赖”靠拢，现有分支与 Agent 仍以当前 001–006 为准，待团队确认后再做目录/编号迁移。

**全功能模块规格**：若要覆盖 Unity 与 Unreal 的**全部**功能域（场景、实体、输入、子系统、Shader、材质、网格、资源、物理、动画、音频、UI、管线、特效、2D、地形、编辑器、网络、XR 等），见 **[tenengine-full-module-spec.md](./tenengine-full-module-spec.md)**（27 模块、功能域映射、完整依赖表与图）。
