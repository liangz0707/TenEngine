# 一帧内 Pass 与接口检查（D3D11 / Vulkan）

以 020 当前一帧调用顺序为基准，对照 D3D11 与 Vulkan 后端实现，检查「所有 Pass 的设置、接口是否被正确调用、一帧所需数据与顺序」是否完整。

---

## 1. 020 一帧调用顺序（代码依据）

以下为 [RenderPipeline.cpp](Engine/TenEngine-020-pipeline/src/RenderPipeline.cpp) Device 任务内、单帧的完整 RHI 调用序列（有 FrameGraph 且 GetPassCount()=2 时）。

### 1.1 帧级（Pass 外）

| 顺序 | 020 调用 | 说明 |
|------|----------|------|
| 1 | `Begin(cmd)` | 开始录制 CommandList |
| 2 | `SetViewport(0, 1, &vp)` | 视口（vpW×vpH） |
| 3 | `SetScissor(0, 1, &scissor)` | 裁剪矩形 |

### 1.2 每个 Pass 内（循环 i = 0 .. GetPassCount()-1）

| 顺序 | 020 调用 | 说明 |
|------|----------|------|
| 4 | `BeginRenderPass(rpDesc)` | rpDesc 在有 SwapChain 时已填 back buffer（colorAttachmentCount=1, Clear/Store） |
| 5 | `BeginOcclusionQuery(0)` | 遮挡查询开始（占位） |
| 6 | 仅 i==0：`ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB)` | 见下「单次 Draw 内」 |
| 7 | `ExecutePass(i, passCtx, cmd)` | 当前为 NoOp |
| 8 | `EndOcclusionQuery(0)` | 遮挡查询结束（占位） |
| 9 | `EndRenderPass()` | 结束 Pass |

### 1.3 单次 Draw 内（ExecuteLogicalCommandBufferOnDeviceThread，每条 LogicalDraw）

| 顺序 | 020 调用 | 说明 |
|------|----------|------|
| D1 | `SetGraphicsPSO(pso)` | 材质 PSO（先于 UB，符合 Vulkan 习惯） |
| D2 | `ub->Bind(cmd, 0)` → `SetUniformBuffer(0, buffer, offset)` | 材质 Uniform |
| D3 | `SetVertexBuffer(0, vb, 0, vertexStride)` | 网格 VB |
| D4 | `SetIndexBuffer(ib, 0, indexFormat)` | 网格 IB |
| D5 | `DrawIndexed(indexCount, instanceCount, firstIndex, 0, firstInstance)` | 绘制 |

### 1.4 帧尾

| 顺序 | 020 调用 | 说明 |
|------|----------|------|
| 10 | `End(cmd)` | 结束录制 |
| 11 | `Submit(cmd, queue)` | 提交到 GPU |
| 12 | `Present()` | 交换链呈现 |

---

## 2. D3D11 后端：接口映射与缺口

### 2.1 映射表（device_d3d11.cpp）

| 020 调用 | D3D11 实现 | 是否真正生效 |
|----------|------------|--------------|
| Begin(cmd) | 清空 recordedList，recording=true | 是 |
| End(cmd) | deferredCtx->FinishCommandList → recordedList | 是 |
| SetViewport | RSSetViewports | 是 |
| SetScissor | RSSetScissorRects | 是 |
| BeginRenderPass(desc) | **(void)desc；无任何调用** | **否** |
| EndRenderPass() | 无调用 | - |
| SetGraphicsPSO(pso) | VSSetShader + PSSetShader | 是（若 pso 非空） |
| SetUniformBuffer(slot, buf, off) | VSSetConstantBuffers(slot,1,&cb) + PSSetConstantBuffers + CSSetConstantBuffers | 是 |
| SetVertexBuffer | IASetVertexBuffers | 是 |
| SetIndexBuffer | IASetIndexBuffer | 是 |
| DrawIndexed | DrawIndexedInstanced | 是 |
| Submit(cmd, queue) | ExecuteCommandList 等（由 Queue 实现） | 是 |

### 2.2 D3D11 一帧必需且顺序

D3D11 规范要求：在调用 `Draw*` 前，必须已绑定至少一个渲染目标（RTV）。绑定通过 `OMSetRenderTargets` 完成。

- **当前问题**：`BeginRenderPass(RenderPassDesc)` 在 D3D11 后端为 `(void)desc`，**未根据 desc 绑定 RTV**。020 已正确填充 `rpDesc.colorAttachments[0].texture`（back buffer），但 008 D3D11 未使用该字段调用 `OMSetRenderTargets`。
- **结果**：若不 elsewhere 绑定 RTV，则所有 Draw 调用在 D3D11 上**未绑定渲染目标**，行为未定义或绘制到错误目标。

### 2.3 D3D11 结论与建议

| 项目 | 状态 | 说明 |
|------|------|------|
| Viewport/Scissor | 已设置且顺序正确 | 在 Pass 循环前设置一次，D3D11 可接受 |
| 渲染目标 (RTV) | **未设置** | BeginRenderPass 未实现；需在 008 D3D11 的 BeginRenderPass 中根据 desc.colorAttachments[0] 取 RTV 并 OMSetRenderTargets |
| PSO (VS/PS) | 已设置 | SetGraphicsPSO → VSSetShader/PSSetShader，顺序在 Draw 前正确 |
| Uniform (CB) | 已设置 | SetUniformBuffer → VSSetConstantBuffers/PSSetConstantBuffers，顺序正确 |
| VB/IB | 已设置 | IASetVertexBuffers / IASetIndexBuffer，顺序正确 |
| DrawIndexed | 已调用 | 参数与 LogicalDraw 一致 |

**已修复**：在 008 D3D11 中已实现：`TextureD3D11` 增加可选 `rtv`；`CreateSwapChain` 创建带 `BIND_RENDER_TARGET` 的纹理并创建 RTV；`BeginRenderPass` 当有 color 附件且纹理带 `rtv` 时调用 `OMSetRenderTargets(1, &t->rtv, nullptr)`。

---

## 3. Vulkan 后端：接口映射与缺口

### 3.1 映射表（device_vulkan.cpp）

| 020 调用 | Vulkan 实现 | 是否真正生效 |
|----------|-------------|--------------|
| Begin(cmd) | vkBeginCommandBuffer | 是 |
| End(cmd) | vkEndCommandBuffer | 是 |
| SetViewport | vkCmdSetViewport | 是 |
| SetScissor | vkCmdSetScissor | 是 |
| BeginRenderPass(desc) | **仅当 desc.colorAttachmentCount==0 时 return；否则无后续调用（注释：需 VkRenderPass/VkFramebuffer，当前跳过）** | **否** |
| EndRenderPass() | 无调用（与 Begin 配对注释） | 否 |
| SetGraphicsPSO(pso) | **(void)pso；无 vkCmdBindPipeline** | **否** |
| SetUniformBuffer | vkUpdateDescriptorSets + vkCmdBindDescriptorSets | 依赖 descriptorSet/pipelineLayout 已创建且有效 |
| SetVertexBuffer | vkCmdBindVertexBuffers | 是 |
| SetIndexBuffer | vkCmdBindIndexBuffer | 是 |
| DrawIndexed | vkCmdDrawIndexed | 是 |
| Submit(cmd, queue) | 提交 VkCommandBuffer | 是 |

### 3.2 Vulkan 一帧必需且顺序

Vulkan 规范要求：

1. **Draw 必须在 Render Pass 内**：`vkCmdDraw*` 必须在 `vkCmdBeginRenderPass` 与 `vkCmdEndRenderPass` 之间。
2. **需先绑定 Pipeline**：通常先 `vkCmdBindPipeline(Graphics)`，再 `vkCmdBindDescriptorSets`，再 BindVertexBuffers/BindIndexBuffer，再 Draw。

当前情况：

- `BeginRenderPass(desc)` 在 colorAttachmentCount>0 时**未调用 vkCmdBeginRenderPass**（缺少由 desc 生成的 VkRenderPass/VkFramebuffer），因此**所有 Draw 在 Vulkan 上落在 Render Pass 外**，违反规范。
- `SetGraphicsPSO` 未调用 `vkCmdBindPipeline`，**图形管线未绑定**。
- `SetUniformBuffer` 会调用 `vkCmdBindDescriptorSets`，但若 `pipelineLayout`/`descriptorSet` 未随 Pipeline 或设备初始化配置，可能无效或崩溃。

### 3.3 Vulkan 结论与建议

| 项目 | 状态 | 说明 |
|------|------|------|
| Viewport/Scissor | 已设置且顺序正确 | 在 Pass 外设置，Vulkan 允许 |
| Render Pass | **未进入** | 需根据 RenderPassDesc 创建/缓存 VkRenderPass 与 VkFramebuffer，在 BeginRenderPass 中调用 vkCmdBeginRenderPass，EndRenderPass 中调用 vkCmdEndRenderPass |
| Pipeline | **未绑定** | SetGraphicsPSO 需调用 vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline) |
| Descriptor (UB) | 依赖 Pipeline/RenderPass | 在 Pipeline 与 Render Pass 正确实现后，SetUniformBuffer 的 BindDescriptorSets 才在有效上下文中 |
| VB/IB | 已设置 | vkCmdBindVertexBuffers / vkCmdBindIndexBuffer，顺序正确 |
| DrawIndexed | 已调用 | 参数正确，但须在 Render Pass 内且已 BindPipeline |

**已修复**：

1. **008 Vulkan BeginRenderPass/EndRenderPass**：设备创建时生成默认 `VkRenderPass`（单 color 附件、Clear/Store）；`TextureVulkan` 增加 `imageView` 与 width/height，`CreateTexture` 中创建 imageView 并设置 extent；`BeginRenderPass` 根据附件纹理创建 `VkFramebuffer` 并调用 `vkCmdBeginRenderPass`，`EndRenderPass` 调用 `vkCmdEndRenderPass` 并销毁本帧 framebuffer。
2. **008 Vulkan SetGraphicsPSO**：`PSOVulkan` 增加 `VkPipeline`；`CreateGraphicsPSO` 中创建完整 VkGraphicsPipeline（与 defaultRenderPass 兼容），`SetGraphicsPSO` 中调用 `vkCmdBindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, p->pipeline)`。
3. descriptorSet / pipelineLayout 由 CommandList 创建时从设备传入，与现有 Pipeline 兼容。

**Viewport/Scissor**：Vulkan 规定在 render pass 外设置的动态 viewport/scissor 不会应用到 pass 内的 draw。020 已在每个 `BeginRenderPass` 之后、录制 draw 之前再次调用 `SetViewport`/`SetScissor`（与帧级相同 vpW/vpH），保证 Vulkan 下 pass 内 draw 使用正确视口与裁剪。

---

## 4. 汇总：一帧接口是否都被正确设置且顺序正确

| 接口/数据 | 020 是否调用 | D3D11 是否生效 | Vulkan 是否生效 | 顺序是否正确 |
|-----------|--------------|----------------|-----------------|--------------|
| Begin/End CommandBuffer | 是 | 是 | 是 | 是 |
| SetViewport / SetScissor | 是 | 是 | 是 | 是（Pass 前） |
| BeginRenderPass / EndRenderPass | 是（且 rpDesc 已填） | **否（RTV 未绑）** | **否（未进 Pass）** | 是 |
| SetGraphicsPSO | 是 | 是 | **否（未 BindPipeline）** | 是（先于 UB/VB/IB） |
| SetUniformBuffer | 是 | 是 | 依赖 layout/set | 是 |
| SetVertexBuffer / SetIndexBuffer | 是 | 是 | 是 | 是 |
| DrawIndexed | 是 | 是 | 是（但 Pass 外无效） | 是 |
| Submit / Present | 是 | 是 | 是 | 是 |

**结论**：

- **020 侧**：一帧内所有 Pass 的设置、以及每条 Draw 的 PSO/UB/VB/IB/Draw 调用与顺序均已按预期执行，且 RenderPassDesc 已从 SwapChain 填充。
- **008 D3D11**：缺「根据 RenderPassDesc 绑定 RTV」；需在 BeginRenderPass 中实现 OMSetRenderTargets。
- **008 Vulkan**：缺「根据 RenderPassDesc 进入 Render Pass」（vkCmdBeginRenderPass/vkCmdEndRenderPass）与「绑定图形 Pipeline」（vkCmdBindPipeline）；需在 008 Vulkan 内实现上述逻辑，一帧所需接口与顺序才完整。
