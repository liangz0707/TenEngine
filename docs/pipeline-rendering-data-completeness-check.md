# 渲染数据流完整性检查：加载 → Device 资源创建

本文档检查从资源加载到设备资源创建、再到绘制调用的完整数据流，标识**尚未完备**的环节。

---

## 1. 数据流概览

```
Load (013) → GetCached
    ↓
PrepareRenderResources (019) → SetDevice + EnsureDeviceResources (011/012/028)
    ↓
IsDeviceReady
    ↓
ConvertToLogicalCommandBuffer (019) → LogicalDraw[] (mesh, material, submeshIndex, instanceCount)
    ↓
ExecuteLogicalCommandBufferOnDeviceThread (020)
    → SetGraphicsPSO / ub->Bind / SetVertexBuffer / SetIndexBuffer / DrawIndexed
```

---

## 2. 渲染数据：完备性

| 数据项 | 来源 | 状态 | 说明 |
|--------|------|------|------|
| **FrameContext** | 020 TickPipeline | ✅ | 提供 viewportWidth/Height、camera 等 |
| **IRenderItemList** | 029 CollectRenderables | ✅ | RenderItem(mesh, material, sortKey) |
| **LogicalDraw** | 019 ConvertToLogicalCommandBuffer | ✅ | mesh, material, submeshIndex, indexCount, firstIndex, instanceCount |
| **RenderPassDesc** | 020 从 SwapChain 填充 | ✅ | colorAttachments[0] = back buffer, Clear/Store |
| **Pass 执行** | graphCapture->ExecutePass(i, passCtx, cmd) | ✅ | 当前 NoOp，主几何在 pass 0 内录制 |

---

## 3. Pass 数据：完备性

| 项目 | 状态 | 说明 |
|------|------|------|
| RenderPassDesc 填充 | ✅ | 020 从 SwapChain 取 back buffer，正确填充 colorAttachments |
| BeginRenderPass / EndRenderPass | ✅ | 008 D3D11/Vulkan 已实现（D3D11 OMSetRenderTargets，Vulkan vkCmdBegin/EndRenderPass） |
| Viewport/Scissor 在 Pass 内 | ✅ | Vulkan 需在 pass 内设置；020 已在每个 BeginRenderPass 后再次调用 |
| ExecuteCallback | ✅ | 按 Pass 执行 ExecutePass，主几何在 pass 0 |
| 多 Pass RT（如 GBuffer） | ⚠️ | 当前仅 SwapChain back buffer；若需自定义 RT，需从 FrameGraph 提供 |

---

## 4. Material：完备性

### 4.1 加载阶段 (011 MaterialResource::Load)

| 项目 | 状态 | 说明 |
|------|------|------|
| 解析 .material JSON | ✅ | ParseMaterialJSON |
| Shader 解析 | ✅ | manager->GetCached(shaderGuid) → IShaderResource::GetShaderHandle |
| Texture 解析 | ✅ | 按 name→GUID 解析，GetCached，填入 textureRefs_（含 set/binding） |
| paramBuffer | ✅ | 从 reflection 构建，按 uniform 成员填充 |

### 4.2 EnsureDeviceResources (011)

| 项目 | 状态 | 说明 |
|------|------|------|
| Texture 依赖 Ensure | ✅ | 对每个 textureRef 调用 SetDevice + EnsureDeviceResources |
| UniformLayout | ✅ | compiler->GetShaderReflection → CreateUniformLayout |
| UniformBuffer | ✅ | CreateUniformBuffer + Update(paramBuffer) |
| **GraphicsPSO** | ❌ | **从未创建**。`graphicsPSO_` 始终为 nullptr，GetGraphicsPSO() 恒返回 nullptr |

### 4.3 PSO 创建缺失

- **008 CreateGraphicsPSO** 已存在，接受 `GraphicsPSODesc{ vertex_shader, vertex_shader_size, fragment_shader, fragment_shader_size }`。
- **011 从未调用** device->CreateGraphicsPSO。
- **010 IShaderCompiler::GetBytecode** 存在，返回单 handle 的 bytecode；但 011 未调用。
- **设计缺口**：  
  - 010 GetBytecode(handle, &size) 返回单 blob；GraphicsPSODesc 需要 vertex + fragment 两个 blob。  
  - 若 shader 为单文件多 entry（如 vs_main/ps_main），需 010 扩展（如 GetBytecode(handle, stage, &size)）或 shader 资源提供 per-stage handle。

**结论**：材质 PSO 未创建，020 调用 `SetGraphicsPSO(matRes->GetGraphicsPSO())` 时传入 nullptr，**无 shader 绑定**，绘制将使用默认/未定义管线。

---

## 5. Texture：完备性

### 5.1 加载与 Ensure

| 项目 | 状态 | 说明 |
|------|------|------|
| 028 TextureResource Load | ✅ | 加载像素数据 |
| 028 EnsureDeviceResources | ✅ | texture::EnsureDeviceResources(h, device) 创建 GPU texture |
| 011 对 textureRef 调用 Ensure | ✅ | MaterialResource::EnsureDeviceResources 中 texRes->SetDevice + EnsureDeviceResources |
| 028 GetDeviceTexture | ✅ | 返回 rhi::ITexture* |

### 5.2 Texture 绑定缺失

| 项目 | 状态 | 说明 |
|------|------|------|
| 011 textureRefs_ | ✅ | 含 slot (set, binding) 与 textureResource |
| **008 ICommandList** | ❌ | **无 SetTexture / SetSampler**。仅有 SetUniformBuffer、SetVertexBuffer、SetIndexBuffer |
| 020 ExecuteLogicalCommandBufferOnDeviceThread | ❌ | 未对材质 textureRefs 做任何绑定 |

**结论**：材质贴图（如 diffuse map）在 GPU 侧从未绑定，shader 无法采样纹理。

---

## 6. PSO：完备性

| 项目 | 状态 | 说明 |
|------|------|------|
| 008 CreateGraphicsPSO | ✅ | D3D11/Vulkan 已实现 |
| 011 创建 PSO | ❌ | 未在 EnsureDeviceResources 中创建 |
| 020 绑定 PSO | ✅ | 调用 SetGraphicsPSO(pso)，但 pso 恒为 nullptr |
| Shader bytecode 来源 | ❌ | 011 未从 010 取 bytecode 传给 CreateGraphicsPSO |

---

## 7. UniformBuffer：完备性

| 项目 | 状态 | 说明 |
|------|------|------|
| 009 IUniformBuffer 创建 | ✅ | 011 EnsureDeviceResources 中 CreateUniformBuffer |
| 009 Update | ✅ | 在 Ensure 内调用一次，paramBuffer 写入 |
| 020 ub->Bind(cmd, 0) | ✅ | SetUniformBuffer(0, buffer, offset) |
| **SetCurrentFrameSlot** | ❌ | 009 IUniformBuffer 有 ring buffer，需 SetCurrentFrameSlot(slot)；020 从未调用 |

**结论**：多帧并发时，所有帧使用 slot 0，可能导致 UB 在被 GPU 读取前被下一帧覆盖。

---

## 8. 汇总：尚未完备的项

| 优先级 | 模块 | 缺口 | 影响 |
|--------|------|------|------|
| **P0** | 011 Material | 未创建 GraphicsPSO | 无 shader，绘制无效 |
| **P0** | 008 RHI | 无 SetTexture/SetSampler | 贴图无法绑定，shader 无法采样 |
| **P1** | 011 Material | 未从 010 取 bytecode 调用 CreateGraphicsPSO | PSO 无法创建 |
| **P1** | 010 Shader | GetBytecode 仅返回单 blob；PSO 需 vertex + fragment | 需扩展或拆分 shader 资源 |
| **P2** | 020 Pipeline | 未调用 ub->SetCurrentFrameSlot(slot) | 多帧并发 UB 可能错乱 |
| **P2** | 020 Pipeline | 未对 material textureRefs 做 SetTexture | 需在 008 增加接口后对接 |

---

## 9. 建议实现顺序

1. **008 增加 SetTexture / SetSampler**（或等价接口），使材质贴图可绑定。
2. **010 扩展**：支持按 stage 取 bytecode（如 GetBytecode(handle, stage, &size)），或 shader 资源提供 vertex/fragment 两个 handle。
3. **011 EnsureDeviceResources**：  
   - 调用 compiler->Compile(handle)（若需）  
   - 取 vertex + fragment bytecode  
   - 调用 device->CreateGraphicsPSO  
   - 赋值 graphicsPSO_
4. **020 ExecuteLogicalCommandBufferOnDeviceThread**：  
   - 对每条 draw 的 material，遍历 textureRefs，调用 cmd->SetTexture(slot.set, slot.binding, tex)  
   - 在 Bind(ub) 前调用 ub->SetCurrentFrameSlot(slot)。

---

## 10. 已完备的环节

- 加载链：013 RequestLoadAsync / GetCached → 011/012/028 Load
- Prepare：019 PrepareRenderResources → 011/012 EnsureDeviceResources
- Mesh：012 EnsureDeviceResources，VB/IB 创建；020 正确 SetVertexBuffer/SetIndexBuffer
- UB 创建与 Update：011 创建并更新 paramBuffer
- Pass 与 RenderPass：020 正确填充 RenderPassDesc，008 D3D11/Vulkan 已实现 BeginRenderPass/EndRenderPass
- 录制顺序：SetGraphicsPSO → ub->Bind → SetVertexBuffer → SetIndexBuffer → DrawIndexed
- 合批与实例化：ConvertToLogicalCommandBuffer 已做 material/mesh/submesh 合并
