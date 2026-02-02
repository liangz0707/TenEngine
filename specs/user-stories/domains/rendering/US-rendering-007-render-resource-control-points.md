# US-rendering-007：渲染资源有显式的控制位置（CreateRenderItem、CollectCommandBuffer、SubmitCommandBuffer、PrepareRenderMaterial/Mesh、CreateDeviceResource/UpdateDeviceResource）

- **标题**：渲染资源有**显式的控制位置**：**创建逻辑渲染资源**（CreateRenderItem）；**创建/收集逻辑上的 CommandBuffer**（CollectCommandBuffer）；**提交到实际 GPU Command**（SubmitCommandBuffer）；**准备渲染资源**（PrepareRenderMaterial、PrepareRenderMesh 等）；**准备/创建/更新 GPU 资源**（CreateDeviceResource、UpdateDeviceResource）。
- **编号**：US-rendering-007

---

## 1. 角色/触发

- **角色**：引擎与游戏侧程序员、渲染/管线开发者
- **触发**：需要**显式的控制位置**来管理渲染资源与命令：**创建逻辑渲染资源**（如 CreateRenderItem）；**创建/收集逻辑上的 CommandBuffer**（CollectCommandBuffer）；**提交到实际 GPU Command**（SubmitCommandBuffer）；**准备渲染资源**（PrepareRenderMaterial、PrepareRenderMesh 等）；**准备/创建/更新 GPU 资源**（CreateDeviceResource、UpdateDeviceResource），便于理解、调试与扩展。

---

## 2. 端到端流程与约定

1. **创建逻辑渲染资源**：**CreateRenderItem** — 创建逻辑上的可渲染项（或由收集阶段产出 RenderItem），供后续 CollectCommandBuffer 与准备渲染资源使用；由 019-PipelineCore 提供。
2. **创建/收集逻辑上的 CommandBuffer**：**CollectCommandBuffer** — 由 RenderItem 列表产出逻辑 CommandBuffer（即 convertToLogicalCommandBuffer）；须在线程 D 调用；由 019-PipelineCore 提供。
3. **提交到实际 GPU Command**：**SubmitCommandBuffer** — 将逻辑 CommandBuffer 提交到实际 GPU（即 submitLogicalCommandBuffer / IDevice::executeLogicalCommandBuffer）；须在线程 D 调用；由 020-Pipeline 与 008-RHI 提供。
4. **准备渲染资源**：**PrepareRenderMaterial**、**PrepareRenderMesh** 等 — 准备材质、网格等对应的 GPU 资源（PSO、纹理绑定、顶点/索引缓冲等）；整体接口 prepareRenderResources；须在线程 D 调用；由 019-PipelineCore 提供。
5. **准备/创建/更新 GPU 资源**：**CreateDeviceResource**、**UpdateDeviceResource** — 创建或更新 Device 侧 GPU 资源（缓冲、纹理、PSO、描述符等）；须在线程 D 调用；由 008-RHI 提供。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 019-PipelineCore | **CreateRenderItem**；**CollectCommandBuffer**（convertToLogicalCommandBuffer）；**PrepareRenderMaterial**、**PrepareRenderMesh**、prepareRenderResources |
| 020-Pipeline | **SubmitCommandBuffer**（submitLogicalCommandBuffer）；协调各控制位置与线程 D |
| 008-RHI | **CreateDeviceResource**、**UpdateDeviceResource**；**SubmitCommandBuffer**（executeLogicalCommandBuffer、submitCommandList）；所有 GPU 操作须在线程 D |

---

## 4. 每模块职责与 I/O

### 019-PipelineCore

- **职责**：提供**创建逻辑渲染资源**（**CreateRenderItem**）；**创建/收集逻辑上的 CommandBuffer**（**CollectCommandBuffer**，即 convertToLogicalCommandBuffer）；**准备渲染资源**（**PrepareRenderMaterial**、**PrepareRenderMesh**、prepareRenderResources）；上述接口须在约定线程（如线程 D 的接口须在线程 D）调用。
- **输入**：场景/Pass 数据、RenderItem 列表、IDevice。
- **输出**：RenderItem、IRenderItemList、ILogicalCommandBuffer；准备就绪的 GPU 资源引用。

### 020-Pipeline

- **职责**：提供**提交到实际 GPU Command**（**SubmitCommandBuffer**，即 submitLogicalCommandBuffer）；协调 CreateRenderItem → CollectCommandBuffer → PrepareRenderMaterial/Mesh → SubmitCommandBuffer 与线程 D。
- **输入**：FrameContext、ILogicalCommandBuffer。
- **输出**：submitLogicalCommandBuffer；将逻辑 CB 交给 RHI 在线程 D 录制并 submit。

### 008-RHI

- **职责**：提供**创建/更新 GPU 资源**（**CreateDeviceResource**、**UpdateDeviceResource**）；**提交到实际 GPU Command**（executeLogicalCommandBuffer、submitCommandList）；所有接口须在线程 D 调用。
- **输入**：资源描述、逻辑 CommandBuffer、ICommandList。
- **输出**：IDevice 资源、提交到 GPU 的命令。

---

## 5. 派生 ABI（与契约/ABI 对齐）

- **019-pipelinecore-ABI**：createRenderItem、PrepareRenderMaterial、PrepareRenderMesh、prepareRenderResources、convertToLogicalCommandBuffer（CollectCommandBuffer）。
- **020-pipeline-ABI**：submitLogicalCommandBuffer（SubmitCommandBuffer）。
- **008-rhi-ABI**：createDeviceResource、updateDeviceResource；executeLogicalCommandBuffer、submitCommandList（SubmitCommandBuffer）。

---

## 6. 验收要点

- 渲染资源有**显式的控制位置**：CreateRenderItem、CollectCommandBuffer、SubmitCommandBuffer、PrepareRenderMaterial、PrepareRenderMesh、CreateDeviceResource、UpdateDeviceResource。
- 各控制位置与现有 ABI 对应明确（CreateRenderItem、convertToLogicalCommandBuffer/CollectCommandBuffer、submitLogicalCommandBuffer/executeLogicalCommandBuffer/SubmitCommandBuffer、prepareRenderMaterial/prepareRenderMesh/prepareRenderResources、createDeviceResource/updateDeviceResource）。
- 须在线程 D 的接口均在文档与契约中标明。
