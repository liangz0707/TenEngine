# Tasks: 008-RHI 完整模块实现

**Input**: Design documents from `specs/008-rhi-fullmodule-005/`  
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/, quickstart.md  

**Tests**: 测试覆盖上游 001-core 能力与第三方库调用（volk/Vulkan、D3D11/D3D12、Metal）；测试可执行文件 link 本模块 target，测试代码主动调用 CreateDevice(Backend)、各后端 API 及上游分配器等。

**Organization**: Tasks grouped by user story (US1–US6) for independent implementation and testing. 实现须基于**全量 ABI**（`specs/_contracts/008-rhi-ABI.md`），禁止 stub/长期 no-op。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: US1–US6 per spec.md
- Include exact file paths. **CMake 任务**：执行前须已澄清**构建根目录**（worktree 路径）；各子模块均使用**源码**引入；cmake 生成后须检查头文件/源文件完整、循环依赖或缺失依赖，有问题须标注或先修复。

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project structure and third-party integration; 构建根目录须在执行 CMake 前与用户澄清。

- [X] T001 Create directory structure per plan: `include/te/rhi/`, `src/device/`, `src/command_list/`, `src/resources/`, `src/pso/`, `src/sync/`, `src/swapchain/`, `src/vulkan/`, `src/d3d12/`, `src/d3d11/`, `src/metal/`, `tests/` at repository root
- [X] T002 [P] Third-party volk: ① 版本选择（docs/third_party/volk.md）；② FetchContent 自动下载（禁止假设已存在）；③ CMake 配置 VOLK_INSTALL OFF；④ 安装；⑤ 编译测试；⑥ 部署进工程；⑦ target_link_libraries(te_rhi PRIVATE volk::volk)、include、TE_RHI_VULKAN 宏，见 docs/third_party/volk.md
- [X] T003 [P] Third-party vulkan-headers: ① 版本选择（docs/third_party/vulkan-headers.md）；② FetchContent 自动下载；③–⑦ 配置、安装、编译测试、部署、target 与 include，与 volk 同用
- [X] T004 [P] Third-party d3d11 (system): ① 版本/Windows SDK；② 无下载（system）；③ WIN32+TE_RHI_D3D11 条件；④–⑦ link d3d11 dxgi、include、宏，见 docs/third_party/d3d11.md
- [X] T005 [P] Third-party d3d12 (system): 同上，d3d12 dxgi，docs/third_party/d3d12.md
- [X] T006 [P] Third-party metal (system): ① 平台/Xcode SDK；② 无下载；③ APPLE+TE_RHI_METAL；④–⑦ link Metal QuartzCore Foundation、include、宏，见 docs/third_party/metal.md
- [X] T007 CMake 顶层：`CMakeLists.txt` 中 tenengine_resolve_my_dependencies("008-rhi")、TE_RHI_VULKAN/D3D12/D3D11/METAL 选项、条件编译后端源文件、target_include_directories、target_link_libraries（001-core、volk、vulkan-headers、d3d11/d3d12/metal 按平台）。**执行前须已澄清构建根目录；各子模块源码引入；生成后检查头文件/源文件完整与依赖**
- [X] T008 [P] Add 001-core dependency resolution in CMake per docs/engine-build-module-convention.md (tenengine_resolve_my_dependencies, 源码引入)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: ABI 头文件与设备工厂；全量 ABI 符号声明；无后端启用时仅允许文档化 Stub 回退。

- [X] T009 [P] Implement ABI types and forward declarations in `include/te/rhi/types.hpp`: Backend, DeviceLimits, QueueType, DeviceFeatures, ResourceState, BufferBarrier, TextureBarrier, IDevice/IQueue/ICommandList/IBuffer/ITexture/ISampler/IPSO/IFence/ISemaphore/ISwapChain 前向声明
- [X] T010 [P] Implement ABI in `include/te/rhi/resources.hpp`: BufferDesc, TextureDesc, SamplerDesc, ViewDesc, ViewHandle, IBuffer, ITexture, ISampler
- [X] T011 [P] Implement ABI in `include/te/rhi/pso.hpp`: GraphicsPSODesc, ComputePSODesc, IPSO
- [X] T012 [P] Implement ABI in `include/te/rhi/queue.hpp`: IQueue (Submit, WaitIdle)
- [X] T013 [P] Implement ABI in `include/te/rhi/sync.hpp`: IFence (Wait, Signal, Reset), ISemaphore, Wait, Signal
- [X] T014 [P] Implement ABI in `include/te/rhi/swapchain.hpp`: SwapChainDesc, ISwapChain (Present, GetCurrentBackBuffer, GetCurrentBackBufferIndex, Resize, GetWidth, GetHeight)
- [X] T015 [P] Implement ABI in `include/te/rhi/command_list.hpp`: Viewport, ScissorRect, LoadOp, StoreOp, kMaxColorAttachments, RenderPassDesc, BufferRegion, TextureRegion, ICommandList 全部成员, Begin, End, Submit
- [X] T016 [P] Implement ABI in `include/te/rhi/device.hpp`: SelectBackend, GetSelectedBackend, CreateDevice, DestroyDevice, IDevice 全部成员
- [X] T017 [P] Implement ABI in `include/te/rhi/descriptor_set.hpp`: DescriptorSetLayoutDesc, DescriptorWrite, IDescriptorSetLayout, IDescriptorSet
- [X] T018 [P] Implement ABI in `include/te/rhi/raytracing.hpp`: RaytracingAccelerationStructureDesc, DispatchRaysDesc
- [X] T019 [P] Declare backend factory APIs in `include/te/rhi/backend_vulkan.hpp`, `backend_d3d12.hpp`, `backend_d3d11.hpp`, `backend_metal.hpp` (CreateDevice* / DestroyDevice* per backend)
- [X] T020 Implement device factory and stub in `src/device/device.cpp`: SelectBackend, GetSelectedBackend, CreateDevice(Backend)/CreateDevice()/DestroyDevice 按 TE_RHI_* 宏分发到各后端；无任何后端启用时使用文档化 Stub 回退（非长期方案，仅用于未启用后端）
- [X] T021 Implement command list free functions in `src/command_list/command_list.cpp`: Begin, End, Submit(cmd, queue), Submit(cmd, queue, signalFence, waitSem, signalSem)

**Checkpoint**: Headers and factory in place; user story implementation can begin

---

## Phase 3: User Story 1 - 创建设备并获取队列 (Priority: P1) — MVP

**Goal**: CreateDevice(Backend)、GetQueue、GetFeatures、GetLimits、DestroyDevice 真实实现；不支持的 Backend 返回 nullptr。

**Independent Test**: CreateDevice(Backend)、GetQueue、GetFeatures、GetLimits、DestroyDevice 不崩溃；不支持 Backend 返回 nullptr。测试须调用上游 001-core 与第三方（如 volk/Vulkan API 或 D3D/Metal API）。

- [X] T022 [P] [US1] Implement Vulkan device and queue in `src/vulkan/device_vulkan.cpp`: VkInstance/VkDevice/VkQueue, GetBackend/GetQueue/GetFeatures/GetLimits, CreateDeviceVulkan/DestroyDeviceVulkan（真实 API，无 stub）
- [X] T023 [P] [US1] Implement D3D12 device and queue in `src/d3d12/device_d3d12.cpp`: ID3D12Device/ID3D12CommandQueue, GetQueue/GetFeatures/GetLimits（真实 API）
- [X] T024 [P] [US1] Implement D3D11 device and queue in `src/d3d11/device_d3d11.cpp`: ID3D11Device/ID3D11DeviceContext, GetQueue/GetFeatures/GetLimits（真实 API）
- [X] T025 [P] [US1] Implement Metal device and queue in `src/metal/device_metal.mm`: MTLDevice/MTLCommandQueue, GetQueue/GetFeatures/GetLimits（真实 API）
- [X] T026 [US1] Add test `tests/device_create.cpp`: CreateDevice(Backend)、GetQueue、GetFeatures、GetLimits、DestroyDevice；按 TE_RHI_* 覆盖已启用后端；调用上游/第三方以验证依赖链

**Checkpoint**: US1 testable independently; at least one backend creates device and queue without crash

---

## Phase 4: User Story 2 - 录制并提交命令 (Priority: P1)

**Goal**: CreateCommandList、Begin/End、Draw、DrawIndexed、Dispatch、ResourceBarrier、SetViewport、SetScissor、BeginRenderPass/EndRenderPass、CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer、Submit with Fence；各后端真实实现。

**Independent Test**: Begin → Draw/Dispatch/ResourceBarrier → End → Submit(cmd, queue) 或 Submit(..., fence)；fence->Wait() 不崩溃。

- [X] T027 [US2] Implement Vulkan command list in `src/vulkan/device_vulkan.cpp`: ICommandList Begin/End, Draw, DrawIndexed, Dispatch, ResourceBarrier (vkCmdPipelineBarrier), SetViewport, SetScissor, BeginRenderPass/EndRenderPass, CopyBuffer/CopyBufferToTexture/CopyTextureToBuffer, Submit path with VkFence
- [X] T028 [US2] Implement D3D12 command list in `src/d3d12/device_d3d12.cpp`: ID3D12GraphicsCommandList 录制、ResourceBarrier、Draw/Dispatch/Copy*、Submit with ID3D12Fence
- [X] T029 [US2] Implement D3D11 command list in `src/d3d11/device_d3d11.cpp`: ID3D11DeviceContext 录制、Draw/Dispatch；ResourceBarrier 文档化空实现（D3D11 无显式屏障）；Submit/Fence 真实
- [X] T030 [US2] Implement Metal command list in `src/metal/device_metal.mm`: MTLCommandBuffer/MTLRenderCommandEncoder 录制、setViewport/setScissorRect、drawIndexedPrimitives、dispatch、blit encoder 拷贝、Submit 与 MTLFence
- [X] T031 [US2] Add test `tests/command_list_submit.cpp`: CreateCommandList, Begin, Draw/DrawIndexed/Dispatch, ResourceBarrier, End, Submit(cmd, queue) and Submit(..., fence), fence->Wait(); 覆盖已启用后端

**Checkpoint**: US2 independently testable; command recording and submit with fence work per backend

---

## Phase 5: User Story 3 - 创建与管理 GPU 资源 (Priority: P1)

**Goal**: CreateBuffer、CreateTexture、CreateSampler、CreateView、Destroy* 各后端真实实现；非法描述符返回 nullptr。

**Independent Test**: CreateBuffer/CreateTexture/CreateSampler 合法描述符非空；size=0/width=0 返回 nullptr；Destroy 后不再使用。

- [X] T032 [US3] Implement Vulkan buffer/texture/sampler and CreateView in `src/vulkan/device_vulkan.cpp`: VkBuffer/VkImage/VkImageView/VkSampler, 真实内存类型与格式映射
- [X] T033 [US3] Implement D3D12 buffer/texture/sampler and CreateView in `src/d3d12/device_d3d12.cpp`: ID3D12Resource, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV/SAMPLER, Create*View
- [X] T034 [US3] Implement D3D11 buffer/texture/sampler and CreateView in `src/d3d11/device_d3d11.cpp`: ID3D11Buffer/ID3D11Texture2D/ID3D11SamplerState/ID3D11ShaderResourceView 等
- [X] T035 [US3] Implement Metal buffer/texture/sampler and CreateView in `src/metal/device_metal.mm`: MTLBuffer/MTLTexture/MTLSamplerState
- [X] T036 [US3] Add test `tests/resources_create.cpp`: CreateBuffer/CreateTexture/CreateSampler, CreateView, Destroy*; 失败路径 size=0/width=0 返回 nullptr；覆盖已启用后端与上游/第三方调用

**Checkpoint**: US3 independently testable; resources created and destroyed per backend

---

## Phase 6: User Story 4 - PSO 与 Shader 绑定 (Priority: P2)

**Goal**: CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO 各后端真实实现（SPIR-V/DXIL/MSL 字节码）；无效 Shader 可返回 nullptr。

**Independent Test**: CreateGraphicsPSO/CreateComputePSO 合法描述符非空；SetShader、Cache 不崩溃。

- [X] T037 [US4] Implement Vulkan PSO in `src/vulkan/device_vulkan.cpp`: VkPipelineLayout/VkPipeline (graphics/compute), VkShaderModule from bytecode, SetShader/Cache
- [X] T038 [US4] Implement D3D12 PSO in `src/d3d12/device_d3d12.cpp`: ID3D12PipelineState, root signature, DXIL bytecode
- [X] T039 [US4] Implement D3D11 PSO in `src/d3d11/device_d3d11.cpp`: ID3D11VertexShader/ID3D11PixelShader/ID3D11ComputeShader, input layout
- [X] T040 [US4] Implement Metal PSO in `src/metal/device_metal.mm`: MTLRenderPipelineState/MTLComputePipelineState, MTLLibrary/MTLFunction from bytecode
- [X] T041 [US4] Add test `tests/pso_create.cpp`: CreateGraphicsPSO/CreateComputePSO, SetShader, Cache, DestroyPSO; 最小合法字节码（如 SPIR-V 头）；覆盖已启用后端

**Checkpoint**: US4 independently testable; PSO creation and shader binding per backend

---

## Phase 7: User Story 5 - 同步与资源屏障 (Priority: P2)

**Goal**: CreateFence(bool)、CreateSemaphore、Wait、Signal、DestroyFence/DestroySemaphore；ICommandList::ResourceBarrier 真实实现（D3D11 为文档化空）。

**Independent Test**: CreateFence(false/true)、Signal、Wait、DestroyFence；ResourceBarrier(0,nullptr,0,nullptr) 合法。

- [X] T042 [US5] Implement Vulkan fence/semaphore in `src/vulkan/device_vulkan.cpp`: VkFence/VkSemaphore, vkWaitForFences, Queue submit signalFence
- [X] T043 [US5] Implement D3D12 fence in `src/d3d12/device_d3d12.cpp`: ID3D12Fence, WaitIdle, Submit signalFence
- [X] T044 [US5] Implement D3D11/Metal fence/semaphore in `src/d3d11/device_d3d11.cpp` and `src/metal/device_metal.mm` (real API where available)
- [X] T045 [US5] Ensure ResourceBarrier in Vulkan/D3D12/Metal uses vkCmdPipelineBarrier/D3D12_RESOURCE_BARRIER/MTLBlitCommandEncoder barrier; D3D11 文档化“无屏障”空实现
- [X] T046 [US5] Add test `tests/sync_fence_semaphore.cpp`: CreateFence, Signal, Wait, Submit with fence, fence->Wait(); 覆盖已启用后端

**Checkpoint**: US5 independently testable; sync and barrier semantics per backend

---

## Phase 8: User Story 6 - SwapChain 与描述符集 (Priority: P3)

**Goal**: CreateSwapChain、Present、GetCurrentBackBuffer、Resize；CreateDescriptorSetLayout、AllocateDescriptorSet、UpdateDescriptorSet、Destroy*；各后端真实实现（无窗口时可离屏 SwapChain）。

**Independent Test**: CreateSwapChain(desc)、Present、Resize；CreateDescriptorSetLayout/AllocateDescriptorSet/UpdateDescriptorSet 非占位（或文档化错误时 nullptr）。

- [X] T047 [US6] Implement Vulkan swapchain and descriptor set in `src/vulkan/device_vulkan.cpp`: VkSwapchainKHR（或离屏 VkImage）、VkDescriptorSetLayout/VkDescriptorPool/VkDescriptorSet、UpdateDescriptorSet 真实 VkWriteDescriptorSet
- [X] T048 [US6] Implement D3D12 swapchain and descriptor heap in `src/d3d12/device_d3d12.cpp`: IDXGISwapChain、ID3D12DescriptorHeap、Create*View 与描述符表
- [X] T049 [US6] Implement D3D11 swapchain and bind slots in `src/d3d11/device_d3d11.cpp`: IDXGISwapChain、ID3D11RenderTargetView/ID3D11ShaderResourceView 等
- [X] T050 [US6] Implement Metal swapchain and argument buffers in `src/metal/device_metal.mm`: CAMetalLayer/MTLTexture 后备、MTLArgumentEncoder/Argument Buffers
- [X] T051 [US6] Add test `tests/swapchain_create.cpp`: CreateSwapChain(desc), Present, Resize, GetWidth/GetHeight; 覆盖已启用后端

**Checkpoint**: US6 independently testable; SwapChain and descriptor set per backend

---

## Phase 9: Polish & Cross-Cutting

**Purpose**: Raytracing (D3D12 real, others explicit unsupported), validation, quickstart

- [X] T052 Implement BuildAccelerationStructure/DispatchRays in `src/d3d12/device_d3d12.cpp` when DXR SDK available (real); in Vulkan/D3D11/Metal implement explicit “unsupported” path (documented, no silent no-op)
- [X] T053 [P] Add raytracing types and stub/unsupported paths in `src/vulkan/device_vulkan.cpp`, `src/d3d11/device_d3d11.cpp`, `src/metal/device_metal.mm` per research.md (explicit unsupported)
- [X] T054 [P] Verify all ABI symbols in `specs/_contracts/008-rhi-ABI.md` are implemented (no missing symbols)
- [X] T055 Run quickstart.md: build from clarified root, run tests, validate upstream and third-party usage in tests
- [X] T056 [P] Documentation: update README or quickstart if needed; ensure engine-build-module-convention §3 (build root) is documented

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: Start immediately; T007/T008 执行前须澄清构建根目录；cmake 生成后检查依赖与头文件/源文件
- **Phase 2 (Foundational)**: Depends on Phase 1 (directories and CMake in place)
- **Phase 3–8 (US1–US6)**: Depend on Phase 2; can proceed sequentially or US2–US6 partially in parallel after US1
- **Phase 9 (Polish)**: Depends on Phases 3–8

### User Story Dependencies

- **US1 (P1)**: After Foundational; no other story required — **MVP**
- **US2 (P1)**: Depends on US1 (device/queue)
- **US3 (P1)**: Depends on US1
- **US4 (P2)**: Depends on US1
- **US5 (P2)**: Depends on US1, US2 (submit with fence)
- **US6 (P3)**: Depends on US1, US3 (resources for swapchain/descriptor)

### Parallel Opportunities

- T002/T003/T004/T005/T006 (third-party) can run in parallel
- T009–T019 (headers) can run in parallel
- T022–T025 (US1 backends) can run in parallel
- T027–T030 (US2 backends), T032–T035 (US3), T037–T040 (US4), T047–T050 (US6) can be parallelized per backend

---

## Implementation Strategy

### MVP First (User Story 1)

1. Complete Phase 1 (Setup) — clarify build root before T007
2. Complete Phase 2 (Foundational)
3. Complete Phase 3 (US1): one or all of Vulkan/D3D12/D3D11/Metal device+queue
4. **STOP and VALIDATE**: Run tests/device_create.cpp; verify CreateDevice/GetQueue/GetFeatures/GetLimits/DestroyDevice

### Incremental Delivery

1. Setup + Foundational → headers and factory ready  
2. US1 → device_create test passes (MVP)  
3. US2 → command_list_submit test passes  
4. US3 → resources_create test passes  
5. US4 → pso_create test passes  
6. US5 → sync_fence_semaphore test passes  
7. US6 → swapchain_create test passes  
8. Polish → raytracing explicit unsupported, quickstart validation  

### Notes

- Every task uses checklist format: `- [ ] Txxx [P?] [USn?] Description with file path`
- CMake/build tasks: 执行前须已澄清构建根目录；各子模块源码引入；生成后检查头文件/源文件与依赖
- Tests must exercise upstream (001-core) and third-party (volk/Vulkan, D3D11/D3D12, Metal) actual API usage
- No long-term stub or silent no-op; D3D11 ResourceBarrier and non-D3D12 raytracing are documented explicit behavior
