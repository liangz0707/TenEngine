# Pipeline 流程与数据完备性检查

本文档描述从 Pass 设置到 RHI 再到实际 Device 调用的完整管线流程，并标注数据准备与潜在缺口。

## 一、流程总览（主线程 → Render 线程 → Device 线程）

```
主线程 TickPipeline(ctx)
  ├─ currentSlot_ 推进
  ├─ FrameGraph 获取/创建
  └─ renderQueue_->Post([...]() {  // 阶段 A：Render 线程
        BuildLogicalPipeline(graph, coreCtx)
        CreateRenderItemList()
        CollectRenderablesToRenderItemList(sceneRef, resourceManager, itemList, frustum, camera)
        RequestStreaming(mesh)  // 对每个 mesh
        deviceQueue_->Post([...]() {  // 阶段 B：Device 线程
          Wait(slotFences_[slot])
          PrepareRenderResources(itemList, rhiDevice)   // ① 数据准备
          readyList = 过滤 IsDeviceReady 的 item
          ConvertToLogicalCommandBuffer(readyList|itemList, pipeline, &logicalCB)  // ② 逻辑命令
          cmd = CreateCommandList(); Begin(cmd)
          SetViewport / SetScissor
          for each pass (或单 pass 无 graph):
            BeginRenderPass(rpDesc)                     // ③ RHI Pass
            [SetViewport/SetScissor 再次]
            BeginOcclusionQuery(0)
            ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, slot)  // ④ 录制 Draw
            graphCapture->ExecutePass(i, passCtx, cmd)  // 可选 Pass 回调
            EndOcclusionQuery(0)
            EndRenderPass()
          End(cmd); Submit(cmd, queue); DestroyCommandList(cmd)
          DestroyLogicalCommandBuffer(logicalCB)
          Signal(slotFences_[slot])
          Present()
        })
      })
```

## 二、各环节说明与完备性

### 1. Pass 设置（019 FrameGraph）

| 项目 | 位置 | 说明 | 完备性 |
|------|------|------|--------|
| AddPass | FrameGraphImpl::AddPass | 返回 IPassBuilder，可 SetExecuteCallback、DeclareRead/Write | ✅ |
| Compile | FrameGraphImpl::Compile | 拓扑排序、依赖推导 | ✅ |
| GetPassCount / ExecutePass | GetPassCount、ExecutePass(executionOrder, ctx, cmd) | 按排序顺序执行；ExecutePass 调用 pass 的 executeCallback | ✅ |
| RenderPassDesc | 020 内填充 | 有 SwapChain 时从 GetCurrentBackBuffer() 填 colorAttachments[0]，loadOp/storeOp/clearColor | ✅ |

主几何绘制在 **第一个 Pass** 的 BeginRenderPass 内通过 `ExecuteLogicalCommandBufferOnDeviceThread(cmd, logicalCB, slot)` 录制；后续 Pass 的 `ExecutePass(i, passCtx, cmd)` 可录制额外绘制（如全屏后处理）。当前无 FrameGraph 时退化为单 Pass。

### 2. 数据准备（019 PrepareRenderResources → 011/012/028）

| 项目 | 位置 | 说明 | 完备性 |
|------|------|------|--------|
| PrepareRenderResources | 019 RenderItem.cpp | 遍历 itemList，对每项调用 PrepareRenderMaterial、PrepareRenderMesh | ✅ |
| PrepareRenderMaterial | 019 → 011 | MaterialResource::SetDevice(device)；MaterialResource::EnsureDeviceResources() | ✅ |
| PrepareRenderMesh | 019 → 012 | MeshResource::GetMeshHandle()；EnsureDeviceResources(mh, device) | ✅ |
| EnsureDeviceResources (011) | MaterialResource.cpp | 贴图链 SetDevice+Ensure；创建 IUniformLayout/IUniformBuffer 并 **Update(paramBuffer_)** 一次；构建 DescriptorSetLayoutDesc；CreateDescriptorSetLayout、AllocateDescriptorSet、defaultSampler、CreateGraphicsPSO(desc, layout) | ⚠️ 见下 |
| IsDeviceReady 过滤 | 020 | readyList 只保留 meshRes->IsDeviceReady() && matRes->IsDeviceReady() 的项；Convert 用 readyList（若有）否则 itemList | ✅ |

**缺口（已修）**：`EnsureDeviceResources` 中只对 UB 调用了一次 `Update(paramBuffer_)`，此时 `currentSlot_` 默认为 0，故 ring buffer 的 slot 1、2 区域从未被写入。在 **UpdateDescriptorSetForFrame** 中应在写 descriptor 前对该 frame 的 slot 上传当前 param：`SetCurrentFrameSlot(frameSlot)` 后调用 `uniformBuffer_->Update(paramBuffer_.data(), totalSize)`，保证本帧所用 slot 的 GPU 数据为最新。已在该处补充 Update 调用。

### 3. 逻辑命令缓冲（019 ConvertToLogicalCommandBuffer）

| 项目 | 位置 | 说明 | 完备性 |
|------|------|------|--------|
| ConvertToLogicalCommandBuffer | LogicalCommandBuffer.cpp | 遍历 items，每项生成 LogicalDraw(mesh, material, submeshIndex, instanceCount)；按 material/mesh/submesh 合并实例 | ✅ |
| LogicalDraw 来源 | RenderItem | material、mesh 来自 RenderItem；submeshIndex 来自 sortKey 低 32 位 | ✅ |
| 排序与合批 | 同文件 | sort 后相邻相同 material/mesh/submesh 合并 instanceCount | ✅ |

### 4. 每 Draw 录制（020 ExecuteLogicalCommandBufferOnDeviceThread）

| 项目 | 位置 | 说明 | 完备性 |
|------|------|------|--------|
| frameSlot | 入参 | 来自当前帧 slot，用于 UB ring 与 descriptor 写入 | ✅ |
| UpdateDescriptorSetForFrame(device, frameSlot) | 011 | SetCurrentFrameSlot；**Update(param)**（已补）；写 UB binding 0（buffer + bufferOffset）+ 各 texture CombinedImageSampler 到 descriptorSet_；device->UpdateDescriptorSet | ✅ |
| SetGraphicsPSO(pso) | 008 | Vulkan：vkCmdBindPipeline + 记录 pipelineLayout；D3D11：VSSetShader/PSSetShader | ✅ |
| BindDescriptorSet(set) | 008 | Vulkan：vkCmdBindDescriptorSets(..., pipelineLayout, ...)；D3D11：按 binding 设置 VSSetConstantBuffers/PSSetConstantBuffers、PSSetShaderResources、PSSetSamplers | ✅ |
| SetVertexBuffer / SetIndexBuffer | 008 | 来自 mesh GetVertexBufferHandle/GetIndexBufferHandle、GetVertexStride、GetIndexFormat | ✅ |
| DrawIndexed | 008 | sub->count, instanceCount, sub->offset, 0, firstInstance | ✅ |

顺序：**UpdateDescriptorSetForFrame → SetGraphicsPSO → BindDescriptorSet → SetVertexBuffer/SetIndexBuffer → DrawIndexed**，符合 Vulkan/D3D11 要求。

### 5. RHI 层（008）

| 项目 | Vulkan | D3D11 | D3D12/Metal |
|------|--------|-------|-------------|
| BeginRenderPass | vkCmdBeginRenderPass，临时创建 framebuffer，clearValue | OMSetRenderTargets(rtv) | 占位 |
| EndRenderPass | vkCmdEndRenderPass，销毁临时 framebuffer | 无操作 | 占位 |
| SetGraphicsPSO | vkCmdBindPipeline；更新 pipelineLayout 供 BindDescriptorSet 使用 | VSSetShader/PSSetShader | 占位 |
| BindDescriptorSet | vkCmdBindDescriptorSets( pipelineLayout, 0, 1, &ds->set ) | 按 layout 绑定 CB/SRV/Sampler | 占位 |

**D3D11 多帧 in-flight**：当前 D3D11 的 descriptor 绑定的是整块 buffer，未使用 `bufferOffset`。若 ring buffer 为单 buffer 多 slot，则 D3D11 下只有 slot 0 区域与绑定语义一致；slot 1/2 需通过 D3D11.1 的带偏移 CBV 或拆成多 buffer 支持。Vulkan 已按 bufferOffset 写 descriptor，无此问题。

### 6. 同步与 Present

| 项目 | 说明 | 完备性 |
|------|------|--------|
| Wait(slotFences_[slot]) | Device 任务开始时等待本 slot 上一帧完成 | ✅ |
| Signal(slotFences_[slot]) | Submit 后在本 slot 上 signal | ✅ |
| Present() | 有 swapChain_ 时调用 GetCurrentBackBuffer 所在交换链的 Present | ✅ |

## 三、数据流小结

- **材质参数**：来自 .material JSON 的 `paramBuffer_`；Ensure 时创建 UB 并 Update 一次；**每帧在 UpdateDescriptorSetForFrame 内对当前 frameSlot 再 Update 一次**，保证该 slot 的 ring 区域为当前参数。
- **纹理**：PrepareRenderResources 时材质 Ensure 会链式 Ensure 贴图；UpdateDescriptorSetForFrame 将各 texture 的 GetDeviceTexture() 与 defaultSampler_ 写入 descriptor。
- **Mesh**：PrepareRenderMesh 对 GetMeshHandle() 调用 EnsureDeviceResources；录制时用 GetVertexBufferHandle/GetIndexBufferHandle、GetSubmesh、GetVertexStride、GetIndexFormat。
- **PSO**：011 Ensure 时按 device 后端选 SPIRV/DXBC，GetBytecodeForStage(vertex/fragment)，CreateGraphicsPSO(desc, descriptorSetLayout_)。

## 四、检查清单（是否完备）

| # | 检查项 | 结果 |
|---|--------|------|
| 1 | Pass 顺序与 ExecutePass 回调是否按 FrameGraph 编译顺序执行 | ✅ |
| 2 | RenderPassDesc 是否从 SwapChain 正确填 back buffer | ✅ |
| 3 | PrepareRenderResources 是否对全部 item 调用 011/012 Ensure | ✅ |
| 4 | 仅 IsDeviceReady 的 item 参与 ConvertToLogicalCommandBuffer | ✅ |
| 5 | 每 draw 前是否 UpdateDescriptorSetForFrame + SetGraphicsPSO + BindDescriptorSet | ✅ |
| 6 | UB 当前帧 slot 是否在写 descriptor 前有最新 param（Update） | ✅ 已补 |
| 7 | Vulkan/D3D11 BeginRenderPass/EndRenderPass、BindDescriptorSet 是否实现 | ✅（D3D12/Metal 为占位） |
| 8 | D3D11 多帧 in-flight 下 CB 偏移（bufferOffset）是否支持 | ⚠️ 未实现，当前为整 buffer 绑定 |
