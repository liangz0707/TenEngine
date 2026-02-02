# Research: 008-RHI 完整模块实现

**Branch**: `008-rhi-fullmodule-005` | **Phase**: 0

## 1. 抽象层与 API 构造（参考 Unity / Unreal）

**Decision**: 采用接口抽象 + 按后端编译期分发；对外仅暴露 `te::rhi` 命名空间下的接口类型（IDevice、IQueue、ICommandList、IBuffer、ITexture、IPSO、IFence、ISemaphore、ISwapChain、IDescriptorSetLayout、IDescriptorSet），不暴露 Vulkan/D3D12/Metal/D3D11 具体类型。

**Rationale**: Unreal 的 FRHICommandList、FDynamicRHI、Unity 的 Graphics API 封装均采用“上层仅见抽象接口、底层按平台/API 实现”的模式；便于下游（RenderCore、Pipeline）与具体图形 API 解耦，且符合 Constitution §VI 契约边界。

**Alternatives considered**:
- 暴露后端句柄（如 VkDevice* 可选）：增加耦合与平台依赖，不符合契约“不暴露后端类型”。
- 纯运行时多态无编译期宏：所有后端代码均链接，体积与分支开销大；当前采用宏选择后端可减小二进制与分支。

---

## 2. 四后端真实实现与“禁止 stub/no-op”的边界

**Decision**:
- **Vulkan / D3D12 / Metal**：Device、Queue、CommandList、Buffer、Texture、Sampler、PSO、Fence、Semaphore、SwapChain、ResourceBarrier、CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer、BeginRenderPass/EndRenderPass、描述符集（各后端用 VkDescriptorSet、ID3D12DescriptorHeap、MTLArgumentEncoder 等真实机制）均调用对应原生 API 实现。
- **D3D11**：Device、Queue、CommandList、Buffer、Texture、Sampler、PSO、Fence、Semaphore、SwapChain 等用 ID3D11Device、ID3D11DeviceContext 等真实实现；ResourceBarrier 在 D3D11 无等价 API，实现为**文档化**的“D3D11 无显式资源屏障”的显式空实现（不属于禁止的“stub”，因该 API 在本后端无语义）。
- **光追（BuildAccelerationStructure / DispatchRays）**：D3D12 在 DXR SDK 可用时真实实现；Vulkan/Metal/D3D11 不提供 DXR 时，实现为**显式“不支持”路径**（可返回或记录错误，或文档化“该后端忽略调用”），不得静默无文档 no-op。

**Rationale**: Constitution 禁止长期 stub 与未声明的代替实现；要求全量 ABI 实现。D3D11 无 ResourceBarrier 语义、部分后端无光追为 API 能力差异，需在设计与文档中明确，而非静默 no-op。

**Alternatives considered**:
- 光追在所有后端都静默 no-op：违反“禁止 no-op”的严格解读，且不利于调试；改为显式不支持路径。

---

## 3. 依赖与构建

**Decision**: 上游仅依赖 001-Core（源码）；Vulkan 使用 volk + vulkan-headers（FetchContent）；D3D11/D3D12/Metal 使用系统 SDK（system）。CMake 按 TE_RHI_VULKAN、TE_RHI_D3D12、TE_RHI_D3D11、TE_RHI_METAL 条件编译对应源文件并链接对应库。

**Rationale**: 与 `docs/engine-build-module-convention.md`、`docs/third_party/` 一致；volk 与 vulkan-headers 源码引入便于版本固定与 CI。

---

## 4. 描述符集在各后端的统一抽象

**Decision**: 统一为 DescriptorSetLayoutDesc、DescriptorWrite、IDescriptorSetLayout、IDescriptorSet；Vulkan 映射到 VkDescriptorSetLayout、VkDescriptorSet、VkDescriptorPool；D3D12 映射到 CBV/SRV/UAV 堆与描述符表；Metal 映射到 MTLArgumentEncoder/Argument Buffers；D3D11 映射到 ID3D11ShaderResourceView 等绑定槽。各后端**真实**实现分配与更新，不返回 nullptr 占位（除非资源不足等可文档化错误）。

**Rationale**: 契约要求描述符集能力；各现代 API 均有等价机制，可实现统一接口下的真实实现。

---

## 5. SwapChain 与窗口

**Decision**: SwapChainDesc 含 windowHandle（可选）、width、height、bufferCount、vsync；无窗口时（windowHandle==nullptr）允许创建“离屏” SwapChain（仅尺寸与缓冲数），Present 可为空操作或返回 true；有窗口时各后端用 VkSurfaceKHR、IDXGISwapChain、MTKView/CAMetalLayer 等真实呈现。

**Rationale**: 与契约及现有 ABI 一致；便于无窗口环境（单元测试、无头渲染）与有窗口环境统一接口。
