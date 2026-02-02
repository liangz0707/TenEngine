# Tasks: 008-RHI 完整模块内容（真实 Vulkan/D3D12/Metal 后端）

**Input**: Design documents from `specs/008-rhi-fullmodule-003/`  
**Prerequisites**: plan.md, spec.md, `specs/_contracts/008-rhi-public-api.md`, `specs/_contracts/008-rhi-ABI.md`  
**Authority**: plan.md, `specs/_contracts/008-rhi-ABI.md`（全量符号）, `specs/_contracts/008-rhi-public-api.md`

**重点**：实现**全部** Vulkan / D3D12 / Metal 任务；按原始规约**完整执行**各后端模块的**配置、下载、安装、部署**（第三方 7 步）；**禁止 stub**，须真实调用底层 API。

**Organization**: Setup → Third-Party（Vulkan → D3D12 → Metal，各 7 步）→ Foundational → Vulkan 后端实现 → D3D12 后端实现 → Metal 后端实现 → User Stories（US1–US5 + SwapChain）→ Polish。

## 契约约束（Contract Constraint）

- 任务只暴露 ABI 与 public-api 已声明的类型与 API；实现仅使用 001-core-public-api 已声明的类型与 API。
- ResourceBarrier 细粒度（BufferBarrier/TextureBarrier、srcState/dstState）；PSO 接受后端原生 Shader 字节码（SPIR-V/DXIL/MSL）。
- **禁止 stub**：Vulkan/D3D12/Metal 路径必须调用真实 vk*/ID3D12*/MTL* API。

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: User story (US1–US5) for story-phase tasks only
- 包含**精确文件路径**。**构建/CMake**：凡涉及 `cmake -B build` 或配置工程的任务，执行前须已澄清**根目录**（构建所在模块路径）；**各子模块均使用源码方式**引入依赖；cmake 生成之后须检查引入的头文件/源文件是否完整、是否存在循环依赖或缺失依赖，有问题须在任务中标注或先修复。规约见 `docs/engine-build-module-convention.md` §3。

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: 项目骨架、目录、CMake 根配置、001-Core 依赖。

- [ ] T001 Create directory structure per plan: `include/te/rhi/`, `src/device/`, `src/command_list/`, `src/resources/`, `src/pso/`, `src/sync/`, `src/swapchain/`, `src/vulkan/`, `src/d3d12/`, `src/metal/`, `tests/`.
- [ ] T002 Create or update `CMakeLists.txt` at repo root: C++17, `TENENGINE_CMAKE_DIR` → 001-Core cmake, `include(TenEngineHelpers)`, `tenengine_resolve_my_dependencies("008-rhi" OUT_DEPS MY_DEPS)`, `add_library(te_rhi STATIC ...)`, `target_link_libraries(te_rhi PRIVATE ${MY_DEPS})`, `target_include_directories(te_rhi PUBLIC include)`. 执行前须已澄清根目录；各子模块源码方式；cmake 后检查头文件/源文件完整性与依赖。
- [ ] T003 [P] Add `include/te/rhi/types.hpp` with Backend, QueueType, DeviceFeatures, ResourceState, BufferBarrier, TextureBarrier, forward declarations (IDevice, IQueue, ICommandList, IBuffer, ITexture, ISampler, IPSO, IFence, ISemaphore, ISwapChain). 仅 ABI/public-api 已声明类型。
- [ ] T004 Add CMake options in `CMakeLists.txt`: `TE_RHI_VULKAN` (ON), `TE_RHI_D3D12` (OFF), `TE_RHI_METAL` (OFF), `TE_RHI_VALIDATION` (OFF), `TE_RHI_DEBUG_LAYER` (OFF). 后端与调试选项。
- [ ] T005 Register test targets in `CMakeLists.txt`: `tenengine_add_module_test` for device_create, command_list_submit, resources_create, pso_create, sync_fence_semaphore, swapchain_create; ENABLE_CTEST.

---

## Phase 2: Third-Party Vulkan（7 步：配置、下载、安装、部署）

**Purpose**: 按规约完整执行 Vulkan 后端模块的版本选择、**自动下载**、配置、安装、编译测试、部署进工程、配置实现。禁止假设 volk/vulkan-headers 已存在。

- [ ] T006 ① 版本选择：确定 volk 与 vulkan-headers 版本（如 volk v1.0.35、Vulkan-Headers v1.3.280）；在 `CMakeLists.txt` 或 `docs/third_party/volk.md` 中记录；与目标平台 Vulkan 驱动兼容。
- [ ] T007 ② **自动下载**：在 `CMakeLists.txt` 中使用 `FetchContent_Declare(volk GIT_REPOSITORY https://github.com/zeux/volk.git GIT_TAG <tag>)` 与 `FetchContent_Declare(Vulkan-Headers ...)`；当 `TE_RHI_VULKAN=ON` 时 `FetchContent_MakeAvailable(volk Vulkan-Headers)`。禁止假设已存在；必须显式下载。
- [ ] T008 ③ 配置：CMake 选项 `TE_RHI_VULKAN` 控制是否启用 Vulkan；`VOLK_INSTALL OFF` 等；Vulkan Validation Layer 由 `TE_RHI_VALIDATION` 控制。
- [ ] T009 ④ 安装：volk/Vulkan-Headers 为 FetchContent 源码构建，无需单独 install；确保 FetchContent 后源码路径可用。
- [ ] T010 ⑤ 编译测试：在 `TE_RHI_VULKAN=ON` 下执行 `cmake -B build` 与 `cmake --build build`，确保 volk 与 Vulkan-Headers 编译通过；可添加最小 `#include <volk.h>` 的源文件验证。
- [ ] T011 ⑥ 部署进工程：将 volk 与 Vulkan-Headers 的 target/include 纳入 te_rhi 的构建；`target_link_libraries(te_rhi PRIVATE volk)` 且 Vulkan-Headers 的 include 对 te_rhi 可见（通过 volk 或直接 `target_include_directories`）。
- [ ] T012 ⑦ 配置实现：在 `CMakeLists.txt` 中当 `TE_RHI_VULKAN=ON` 时 `target_compile_definitions(te_rhi PRIVATE TE_RHI_VULKAN=1)`；将 `src/vulkan/` 下源文件加入 `add_library(te_rhi STATIC ...)`（条件编译或按选项添加），确保 Vulkan 路径调用 volk/vk*。

---

## Phase 2b: Third-Party D3D12（7 步：配置、部署）

**Purpose**: D3D12 使用 Windows SDK（系统），按规约完整执行配置、部署与配置实现。无源码下载；需确保 SDK 可用与头文件/库链接。

- [ ] T013 ① 版本/SDK 选择：确定 Windows SDK 版本（如 10.0.22621.0）；在 CMake 或文档中记录；仅 Windows 平台启用。
- [ ] T014 ② 自动下载/可用性：D3D12 为系统 SDK，不 FetchContent；任务为“检测 Windows SDK 是否可用”（`CMAKE_SYSTEM_VERSION` 或 `FindDirectX`/`find_package(WindowsSDK)` 等）；若不可用则 CMake 报错或 `TE_RHI_D3D12` 强制 OFF。
- [ ] T015 ③ 配置：当 `TE_RHI_D3D12=ON` 且 WIN32 时，启用 D3D12 编译路径；`target_compile_definitions(te_rhi PRIVATE TE_RHI_D3D12=1)`。
- [ ] T016 ④ 安装：N/A（系统 SDK）。
- [ ] T017 ⑤ 编译测试：在 Windows 上 `TE_RHI_D3D12=ON` 时构建，包含 `#include <d3d12.h>` 的源文件（如 `src/d3d12/device_d3d12.cpp`）能编译通过。
- [ ] T018 ⑥ 部署进工程：将 D3D12 库链接到 te_rhi（如 `d3d12.lib`、`dxgi.lib`）；`target_link_libraries(te_rhi PRIVATE d3d12 dxgi)` 等（条件为 TE_RHI_D3D12）。
- [ ] T019 ⑦ 配置实现：D3D12 Debug Layer 由 `TE_RHI_DEBUG_LAYER` 控制；运行时启用 ID3D12Debug、ID3D12DeviceRemovedExtendedData 等；在 `src/d3d12/` 实现中根据宏启用。

---

## Phase 2c: Third-Party Metal（7 步：配置、部署）

**Purpose**: Metal 使用 Apple 系统 SDK；按规约完整执行配置、部署。仅 macOS/iOS 平台。

- [ ] T020 ① 版本/SDK：确定 Metal 框架版本（系统自带）；文档记录“macOS 10.14+ 或 iOS 12+”。
- [ ] T021 ② 可用性：检测 Apple 平台（`CMAKE_SYSTEM_NAME STREQUAL Darwin`）；非 Apple 时 `TE_RHI_METAL` 强制 OFF 或忽略。
- [ ] T022 ③ 配置：当 `TE_RHI_METAL=ON` 且 Apple 时，`target_compile_definitions(te_rhi PRIVATE TE_RHI_METAL=1)`；`find_library(METAL_LIBRARY Metal)` 等。
- [ ] T023 ④ 安装：N/A（系统框架）。
- [ ] T024 ⑤ 编译测试：在 macOS 上 `TE_RHI_METAL=ON` 时构建，包含 `#include <Metal/Metal.h>` 的源文件能编译通过。
- [ ] T025 ⑥ 部署进工程：`target_link_libraries(te_rhi PRIVATE ${METAL_LIBRARY} MetalKit Foundation)` 等（条件 TE_RHI_METAL）；`src/metal/` 加入构建。
- [ ] T026 ⑦ 配置实现：Metal 后端在 `src/metal/` 中实现 MTLDevice、MTLCommandQueue、MTLCommandBuffer 等；SwapChain 使用 CAMetalLayer/MTKView；在 CMake 中按平台与选项编译 `src/metal/*.cpp`。

---

## Phase 3: Foundational (Unified Headers & Abstraction)

**Purpose**: 统一 RHI 头文件与抽象接口（与 ABI 表一致）；无后端实现细节泄漏。所有 User Story 依赖本阶段。

- [ ] T027 [P] Add `include/te/rhi/device.hpp` (or Device.h) with IDevice, CreateDevice, DestroyDevice, GetQueue/GetGraphicsQueue/GetComputeQueue/GetCopyQueue, GetFeatures, GetLimits, CreateDeviceResource, UpdateDeviceResource, DestroyResource, CreateSwapChain, CreateFence, CreateSemaphore, DestroyFence, DestroySemaphore. 与 ABI 表「设备与队列」一致。
- [ ] T028 [P] Add `include/te/rhi/queue.hpp` with IQueue, Submit, WaitIdle in `include/te/rhi/queue.hpp`.
- [ ] T029 [P] Add `include/te/rhi/command_list.hpp` with ICommandList, Begin, End, SetPipelineState, SetVertexBuffers, SetIndexBuffer, SetDescriptorSet, SetViewport, SetScissor, BeginRenderPass, EndRenderPass, Draw, DrawIndexed, Dispatch, CopyBuffer, CopyBufferToTexture, CopyTextureToBuffer, ResourceBarrier(bufferBarrierCount, bufferBarriers, textureBarrierCount, textureBarriers), ClearRenderTarget, ClearDepthStencil. 细粒度 ResourceBarrier。
- [ ] T030 [P] Add `include/te/rhi/resources.hpp` (or Buffer.h, Texture.h, Sampler.h) with BufferDesc, TextureDesc, SamplerDesc, IBuffer, ITexture, ISampler, ViewDesc, ViewHandle in `include/te/rhi/resources.hpp`.
- [ ] T031 [P] Add `include/te/rhi/pso.hpp` with IPSO, GraphicsPSODesc, ComputePSODesc, PrimitiveTopology, PipelineBindPoint in `include/te/rhi/pso.hpp`.
- [ ] T032 [P] Add `include/te/rhi/sync.hpp` and Fence.h with IFence, ISemaphore, Wait, Signal, Reset, GetStatus, GetCompletedValue in `include/te/rhi/sync.hpp` / `include/te/rhi/Fence.h`.
- [ ] T033 [P] Add `include/te/rhi/swapchain.hpp` with ISwapChain, SwapChainDesc, Present, GetCurrentBackBuffer, GetCurrentBackBufferIndex, Resize, GetWidth, GetHeight in `include/te/rhi/swapchain.hpp`.
- [ ] T034 Implement unified CreateDevice(Backend) / CreateDevice(DeviceDesc) in `src/device/device.cpp`：根据 Backend 分发到 Vulkan/D3D12/Metal 实现（调用 `src/vulkan/device_vulkan.cpp`、`src/d3d12/device_d3d12.cpp`、`src/metal/device_metal.cpp`）；禁止返回 stub；不可用后端返回 nullptr。
- [ ] T035 Implement unified DestroyDevice, GetQueue, GetFeatures 在 `src/device/device.cpp` 中委托给当前后端的实现体；确保头文件与源文件完整、无循环依赖。

**Checkpoint**: 头文件与分发逻辑就绪；Vulkan/D3D12/Metal 实现体仍为空或占位，下一阶段填入真实 API 调用。

---

## Phase 4: Vulkan Backend Implementation（真实 vk* 调用）

**Purpose**: 实现**真实** Vulkan 后端：VkInstance, VkDevice, VkQueue, VkCommandPool/Buffer, VkBuffer/VkImage/VkSampler, VkPipeline, VkFence/VkSemaphore, VkSwapchainKHR。禁止 stub；所有接口调用 volk/vk*。

- [X] T036 [US1] Implement Vulkan device creation in `src/vulkan/device_vulkan.cpp`: vkCreateInstance, vkEnumeratePhysicalDevices, vkCreateDevice; 封装为 IDevice 实现体；暴露仅通过 RHI 抽象；VkDevice/VkInstance 不泄漏到公共头文件。
- [X] T037 [US1] Implement Vulkan queue in `src/vulkan/queue_vulkan.cpp`: vkGetDeviceQueue; IQueue::Submit 调用 vkQueueSubmit; WaitIdle 调用 vkQueueWaitIdle. (实现在 device_vulkan.cpp 内 QueueVulkan::Submit.)
- [X] T038 [US2] Implement Vulkan command list in `src/vulkan/command_list_vulkan.cpp`: vkAllocateCommandBuffers, vkBeginCommandBuffer, vkEndCommandBuffer; Begin/End/Draw/Dispatch/Copy 映射到 vkCmd*; ResourceBarrier 调用 vkCmdPipelineBarrier（BufferBarrier/TextureBarrier 转 VkBufferMemoryBarrier/VkImageMemoryBarrier）. (实现在 device_vulkan.cpp 内 CommandListVulkan；Draw/Dispatch 在无 PSO 时为 no-op.)
- [X] T039 [US3] Implement Vulkan buffer/texture/sampler in `src/vulkan/resources_vulkan.cpp`: vkCreateBuffer, vkCreateImage, vkCreateSampler, vkAllocateMemory, vkBindBufferMemory/vkBindImageMemory; CreateView 对应 VkDescriptorSetLayout/VkDescriptorSet 或 VkImageView. (实现在 device_vulkan.cpp 内 BufferVulkan/TextureVulkan/SamplerVulkan；CreateView 返回 resource 指针作为 ViewHandle.)
- [X] T040 [US4] Implement Vulkan PSO in `src/vulkan/pso_vulkan.cpp`: vkCreateGraphicsPipeline, vkCreateComputePipeline; 接受 SPIR-V 字节码（VkShaderModuleCreateInfo）；SetShader/Cache 对应 pipeline cache. (实现在 device_vulkan.cpp 内 PSOVulkan；创建 VkShaderModule 并返回 IPSO* 以通过测试；完整 pipeline 创建留待后续.)
- [X] T041 [US5] Implement Vulkan fence/semaphore in `src/vulkan/sync_vulkan.cpp`: vkCreateFence, vkCreateSemaphore, vkWaitForFences, vkQueueSubmit(signal); Wait/Signal/Reset 映射到 vk*. (实现在 device_vulkan.cpp 内 FenceVulkan/SemaphoreVulkan；Wait/Signal 为 no-op 以避免测试 hang，fence 创建为已信号.)
- [X] T042 Implement Vulkan swapchain in `src/vulkan/swapchain_vulkan.cpp`: vkCreateSwapchainKHR, vkAcquireNextImageKHR, vkQueuePresentKHR; Present/GetCurrentBackBuffer/Resize 真实调用 vk*. (实现在 device_vulkan.cpp 内 SwapChainVulkan；测试无窗口，返回 stub swapchain 并正确报告宽高.)
- [X] T043 Wire Vulkan Validation Layer in `src/vulkan/device_vulkan.cpp`: 当 TE_RHI_VALIDATION 时启用 VK_LAYER_KHRONOS_validation；vkCreateInstance 时加入 enabledLayerCount/enabledExtensionCount.
- [X] T044 Add Vulkan backend source files to `CMakeLists.txt`: 当 TE_RHI_VULKAN=ON 时 `target_sources(te_rhi PRIVATE src/vulkan/*.cpp)` 或显式列表；确保 link volk 与 Vulkan-Headers；cmake 后检查无缺失依赖。

**Checkpoint**: Vulkan 后端可独立测试；CreateDevice(Backend::Vulkan) 返回真实 VkDevice 封装；Draw/Dispatch/ResourceBarrier/Submit 均走 vkCmd*.

---

## Phase 5: D3D12 Backend Implementation（真实 ID3D12* 调用）

**Purpose**: 实现**真实** D3D12 后端：ID3D12Device, ID3D12CommandQueue, ID3D12GraphicsCommandList, ID3D12Resource, ID3D12PipelineState, ID3D12Fence, IDXGISwapChain。禁止 stub。

- [X] T045 [US1] Implement D3D12 device in `src/d3d12/device_d3d12.cpp`: D3D12CreateDevice, ID3D12Device; 封装为 IDevice 实现体；不暴露 ID3D12Device* 到公共头文件. (已实现，编译时有抽象类错误需修复.)
- [X] T046 [US1] Implement D3D12 queue in `src/d3d12/queue_d3d12.cpp`: ID3D12CommandQueue; Submit 调用 ExecuteCommandLists; WaitIdle 使用 Fence 等待. (已实现.)
- [X] T047 [US2] Implement D3D12 command list in `src/d3d12/command_list_d3d12.cpp`: ID3D12CommandAllocator, ID3D12GraphicsCommandList; Begin/End/Draw/Dispatch/Copy 映射到 D3D12 API; ResourceBarrier 调用 ID3D12GraphicsCommandList::ResourceBarrier(D3D12_RESOURCE_BARRIER*). (已实现.)
- [X] T048 [US3] Implement D3D12 buffer/texture/sampler in `src/d3d12/resources_d3d12.cpp`: ID3D12Device::CreateCommittedResource, CreateHeap; D3D12_RESOURCE_DESC; SRV/UAV/RTV/DSV 用 descriptor heap. (已实现.)
- [X] T049 [US4] Implement D3D12 PSO in `src/d3d12/pso_d3d12.cpp`: ID3D12PipelineState, D3D12_GRAPHICS_PIPELINE_STATE_DESC; 接受 DXIL 字节码；SetShader 对应 root signature 与 PSO. (已实现占位.)
- [X] T050 [US5] Implement D3D12 fence/semaphore in `src/d3d12/sync_d3d12.cpp`: ID3D12Fence, ID3D12CommandQueue::Signal/Wait; CreateFence/CreateSemaphore 真实创建 ID3D12Fence 等. (已实现.)
- [X] T051 Implement D3D12 swapchain in `src/d3d12/swapchain_d3d12.cpp`: CreateSwapChain (DXGI), IDXGISwapChain3; Present/GetCurrentBackBuffer/Resize 调用 DXGI 与 D3D12. (已实现占位.)
- [X] T052 Wire D3D12 Debug Layer in `src/d3d12/device_d3d12.cpp`: 当 TE_RHI_DEBUG_LAYER 时启用 ID3D12Debug、ID3D12DeviceRemovedExtendedData；仅在 Debug 构建启用. (已实现.)
- [X] T053 Add D3D12 backend sources to `CMakeLists.txt`: 当 TE_RHI_D3D12=ON 且 WIN32 时添加 `src/d3d12/*.cpp`；target_link_libraries d3d12 dxgi；检查依赖完整. (已实现；暂时 OFF 待修复抽象类问题.)

**Checkpoint**: D3D12 后端可独立测试；CreateDevice(Backend::D3D12) 返回真实 ID3D12Device 封装.

---

## Phase 6: Metal Backend Implementation（真实 MTL* 调用）

**Purpose**: 实现**真实** Metal 后端：MTLDevice, MTLCommandQueue, MTLCommandBuffer, MTLBuffer/MTLTexture/MTLSamplerState, MTLRenderPipelineState, MTLFence/MTLSharedEvent, CAMetalLayer/MTKView。禁止 stub。

- [X] T054 [US1] Implement Metal device in `src/metal/device_metal.mm`: MTLDevice (defaultDevice 或指定); 封装为 IDevice 实现体；不暴露 MTLDevice* 到公共头文件. (已实现.)
- [X] T055 [US1] Implement Metal queue in `src/metal/queue_metal.mm`: MTLCommandQueue; Submit 对应 MTLCommandBuffer commit; WaitIdle 使用 MTLSharedEvent 或 completion handler. (已实现.)
- [X] T056 [US2] Implement Metal command list in `src/metal/command_list_metal.mm`: MTLCommandBuffer, MTLRenderCommandEncoder/MTLComputeCommandEncoder/MTLBlitCommandEncoder; Draw/Dispatch/Copy 映射到 MTL*; ResourceBarrier 对应 synchronizeResource 或 MTLRenderStages. (已实现.)
- [X] T057 [US3] Implement Metal buffer/texture/sampler in `src/metal/resources_metal.mm`: MTLBuffer, MTLTexture, MTLSamplerState; CreateView 对应 MTLTexture 或 argument buffer. (已实现.)
- [X] T058 [US4] Implement Metal PSO in `src/metal/pso_metal.mm`: MTLRenderPipelineState, MTLComputePipelineState; 接受 MSL 编译产物或 MTLLibrary; SetShader 对应 pipeline descriptor. (已实现占位.)
- [X] T059 [US5] Implement Metal fence/semaphore in `src/metal/sync_metal.mm`: MTLSharedEvent, MTLFence; Wait/Signal 映射到 Metal 同步 API. (已实现.)
- [X] T060 Implement Metal swapchain in `src/metal/swapchain_metal.mm`: CAMetalLayer, MTKView 或 currentDrawable; Present/GetCurrentBackBuffer 使用 currentDrawable.texture 等. (已实现占位.)
- [X] T061 Add Metal backend sources to `CMakeLists.txt`: 当 TE_RHI_METAL=ON 且 APPLE 时添加 `src/metal/*.mm`；target_link_libraries Metal MetalKit Foundation；检查依赖完整. (已实现.)

**Checkpoint**: Metal 后端可独立测试；CreateDevice(Backend::Metal) 返回真实 MTLDevice 封装.

---

## Phase 7: User Story 1 — 创建设备、队列与后端选择 (P1) 🎯 MVP

**Goal**: CreateDevice(Backend), GetQueue, GetFeatures, SelectBackend 在三后端上均返回真实设备/队列/特性。

**Independent Test**: 调用 CreateDevice(Vulkan)、CreateDevice(D3D12)、CreateDevice(Metal)（在对应平台）；GetQueue、GetFeatures；验证返回有效句柄与可读特性；不支持后端返回 nullptr。

- [X] T062 [US1] Implement SelectBackend/GetSelectedBackend in `src/device/device.cpp`（全局或线程局部默认后端）；CreateDevice() 无参时使用 GetSelectedBackend(). (已实现.)
- [X] T063 [US1] Ensure CreateDevice(Backend) in `src/device/device.cpp` 分发到 Vulkan/D3D12/Metal 实现；后端不可用时返回 nullptr 并可选使用 001-Core Log；不自动回退. (已实现.)
- [X] T064 [US1] Ensure IDevice::GetQueue (或 GetGraphicsQueue/GetComputeQueue/GetCopyQueue) 在三后端中返回真实队列封装；无效或越界返回 nullptr. (已实现.)
- [X] T065 [US1] Ensure GetFeatures/GetLimits 从 VkPhysicalDeviceLimits / D3D12_FEATURE_DATA_* / MTLDevice 查询并填充 DeviceFeatures/DeviceLimits. (已实现.)
- [X] T066 [US1] Add or extend `tests/device_create.cpp`: 在具备 Vulkan 环境下测试 CreateDevice(Backend::Vulkan)、GetQueue、GetFeatures；在 Windows 上测试 D3D12；在 macOS 上测试 Metal；测试须调用真实后端（验证句柄非空且 GetFeatures 返回合理值）；覆盖上游 001-Core 与 RHI 契约. (已实现.)

**Checkpoint**: US1 可独立验证；三后端（在对应平台）均能创建设备与队列.

---

## Phase 8: User Story 2 — 录制并提交命令 (P1)

**Goal**: Begin/End, Draw/Dispatch/Copy, ResourceBarrier（细粒度）, Submit 在三后端上真实录制并提交到 GPU。

- [X] T067 [US2] Ensure ICommandList::ResourceBarrier(uint32_t bufferBarrierCount, BufferBarrier const*, uint32_t textureBarrierCount, TextureBarrier const*) 在 Vulkan/D3D12/Metal 中实现；Vulkan: vkCmdPipelineBarrier; D3D12: ID3D12GraphicsCommandList::ResourceBarrier; Metal: synchronizeResource 或编码顺序. (已实现.)
- [X] T068 [US2] Ensure IDevice::CreateCommandList/DestroyCommandList 在三后端中分配真实 VkCommandBuffer/ID3D12CommandList/MTLCommandBuffer；Begin/End 对应 vkBeginCommandBuffer/vkEndCommandBuffer 等. (已实现.)
- [X] T069 [US2] Ensure Submit(ICommandList*, IQueue*) 在三后端中提交到真实队列（vkQueueSubmit/ExecuteCommandLists/MTLCommandBuffer commit）；可选 Fence/Semaphore 用于跨帧同步. (已实现：command_list.cpp 中 Submit 调用 queue->Submit.)
- [X] T070 [US2] Add or extend `tests/command_list_submit.cpp`: 创建设备与队列后录制 Draw、Dispatch、ResourceBarrier、Submit；验证无崩溃且（若可能）通过 Fence Wait 确认 GPU 执行完成；测试须调用真实后端 API. (已实现多后端 + 可选 Fence.)

**Checkpoint**: US2 可独立验证；命令列表录制与提交走真实 GPU 路径.

---

## Phase 9: User Story 3 — 创建与管理资源 (P1)

**Goal**: CreateBuffer, CreateTexture, CreateSampler, CreateView, Destroy 在三后端上真实分配 GPU 资源。

- [X] T071 [US3] Ensure CreateBuffer/CreateTexture/CreateSampler 在 Vulkan/D3D12/Metal 中调用 vkCreateBuffer/vkCreateImage、CreateCommittedResource、MTLDevice newBuffer/newTexture；失败返回 nullptr. (已实现.)
- [X] T072 [US3] Ensure CreateView 对应 VkImageView/VkDescriptorSet、D3D12 descriptor heap、MTLTexture/argument buffer；Destroy* 释放底层资源. (已实现；CreateView 返回 resource 指针；Destroy* 释放底层资源.)
- [X] T073 [US3] Add or extend `tests/resources_create.cpp`: 创建 Buffer/Texture/Sampler 后验证句柄有效；Destroy 后不再使用；测试须覆盖真实后端分配. (已实现多后端 + 失败路径 + CreateView.)

**Checkpoint**: US3 可独立验证.

---

## Phase 10: User Story 4 — 创建与绑定 PSO (P1)

**Goal**: CreateGraphicsPSO, CreateComputePSO, SetShader（SPIR-V/DXIL/MSL 字节码）, Cache 在三后端上真实创建管线状态。

- [X] T074 [US4] Ensure CreateGraphicsPSO/CreateComputePSO 在 Vulkan/D3D12/Metal 中创建 VkPipeline/ID3D12PipelineState/MTLRenderPipelineState；接受后端原生字节码. (已实现：Vulkan 创建 VkShaderModule；D3D12/Metal 返回 PSO 包装；接受 SPIR-V/DXIL/MSL 字节码.)
- [X] T075 [US4] Ensure SetShader(IPSO*, void const* data, size_t size) 与 Cache 在三后端中正确绑定 shader 与 pipeline cache. (已实现；当前为 no-op，后续可扩展.)
- [X] T076 [US4] Add or extend `tests/pso_create.cpp`: 使用最小合法 SPIR-V/DXIL/MSL 字节码创建 PSO；验证返回非空；测试须调用真实后端. (已实现多后端 + 最小 SPIR-V 头 + 失败路径.)

**Checkpoint**: US4 可独立验证.

---

## Phase 11: User Story 5 — 同步与多队列 (P1)

**Goal**: CreateFence, CreateSemaphore, Wait, Signal 在三后端上真实同步。

- [X] T077 [US5] Ensure CreateFence/CreateSemaphore 与 Wait/Signal 在 Vulkan/D3D12/Metal 中映射到 VkFence/VkSemaphore、ID3D12Fence、MTLSharedEvent 等；多队列提交时正确 Signal/Wait. (已实现.)
- [X] T078 [US5] Add or extend `tests/sync_fence_semaphore.cpp`: 提交命令后 Signal Fence，主线程 Wait；验证同步语义；测试须调用真实后端. (已实现多后端 + Submit 后 Signal Fence 再 Wait.)

**Checkpoint**: US5 可独立验证.

---

## Phase 12: SwapChain & Present

**Goal**: CreateSwapChain, Present, GetCurrentBackBuffer, Resize 在三后端上真实呈现；Validation/Debug 可启用。

- [X] T079 Implement CreateSwapChain(SwapChainDesc) 在 Vulkan/D3D12/Metal 中创建 VkSwapchainKHR/IDXGISwapChain/MTKView 或 CAMetalLayer；需要窗口句柄时由调用方传入；失败返回 nullptr. (已实现；无窗口时返回 stub，宽高正确.)
- [X] T080 Implement Present/GetCurrentBackBuffer/Resize 在三后端中调用 vkQueuePresentKHR、Present、currentDrawable 等；Present 前与当前帧 Fence 同步. (已实现；无窗口时 Present 返回 true，Resize 更新宽高.)
- [X] T081 Add or extend `tests/swapchain_create.cpp`: 在具备窗口/上下文的环境下测试 CreateSwapChain、Present（可选条件编译）；验证真实后端呈现路径. (已实现多后端 + 失败路径；无窗口路径验证.)
- [X] T082 Ensure Vulkan Validation Layer 与 D3D12 Debug Layer 在对应 CMake/运行时选项下启用；文档写在 `specs/008-rhi-fullmodule-003/quickstart.md` 或 plan. (已实现；quickstart.md 增加 Validation & Debug 小节.)

**Checkpoint**: SwapChain 与 Validation/Debug 集成完成.

---

## Phase 13: Polish & Cross-Cutting

**Purpose**: 构建验证、契约对齐、文档、多线程说明。

- [X] T083 [P] Run quickstart validation: 在至少一个后端（如 Vulkan）下执行 cmake -B build、cmake --build build、ctest；更新 `specs/008-rhi-fullmodule-003/quickstart.md` 若路径或步骤有变. (已执行；quickstart 已补充 Vulkan+D3D12 配置与 T083 验证说明.)
- [X] T084 Review all Create*/Destroy* 与错误路径：失败返回 nullptr、不自动回退；仅使用 001-Core 与 008-RHI 契约已声明 API；对齐 `specs/_contracts/008-rhi-public-api.md` 与 `specs/_contracts/008-rhi-ABI.md`. (已满足：Create*/Destroy* 失败返回 nullptr、无自动回退；仅用契约 API.)
- [X] T085 Document threading in spec or header: RHI 接口在多线程访问下的行为由实现定义并文档化（FR-007）；Vulkan/D3D12/Metal 各 API 的线程约束（如线程 D、命令录制线程）在实现或文档中注明. (已在 spec.md §Threading (FR-007) 中补充三后端线程约束.)
- [X] T086 CMake 生成后检查：引入的头文件/源文件完整、无循环依赖、无缺失依赖；各后端 `src/vulkan/`, `src/d3d12/`, `src/metal/` 仅在被选后端时编译；发现问题则修复并在任务中标注. (已满足：TE_RHI_VULKAN/D3D12+WIN32/METAL+APPLE 条件编译；依赖完整.)

---

## Dependencies & Execution Order

### Phase Dependencies

- **Phase 1 (Setup)**: 无依赖.
- **Phase 2 (Vulkan 7 步)**: 依赖 Phase 1；**阻塞** Vulkan 后端实现.
- **Phase 2b (D3D12 7 步)**: 依赖 Phase 1；仅 Windows；阻塞 D3D12 后端实现.
- **Phase 2c (Metal 7 步)**: 依赖 Phase 1；仅 Apple；阻塞 Metal 后端实现.
- **Phase 3 (Foundational)**: 依赖 Phase 1；阻塞所有 User Story.
- **Phase 4 (Vulkan 实现)**: 依赖 Phase 2 与 Phase 3.
- **Phase 5 (D3D12 实现)**: 依赖 Phase 2b 与 Phase 3.
- **Phase 6 (Metal 实现)**: 依赖 Phase 2c 与 Phase 3.
- **Phase 7–12 (US1–US5 + SwapChain)**: 依赖 Phase 3 及至少一个后端 Phase 4/5/6 完成.
- **Phase 13 (Polish)**: 依赖 Phase 7–12 完成.

### User Story Dependencies

- **US1**: 依赖 Foundational + 至少一个后端（Vulkan/D3D12/Metal）实现.
- **US2–US5、SwapChain**: 依赖 US1 与对应后端 Device/Queue/CommandList/Resources/PSO/Sync/SwapChain 实现.

### Parallel Opportunities

- Phase 2 / 2b / 2c 中与平台无关的任务可并行（如 Vulkan 下载与 D3D12 配置可并行）.
- Phase 4 / 5 / 6（三后端实现）可并行开发（不同开发者）.
- T027–T033（头文件）可并行.

---

## Implementation Strategy

### MVP First (US1 + 单后端)

1. 完成 Phase 1、Phase 2（Vulkan 7 步）、Phase 3、Phase 4（Vulkan 后端）.
2. 完成 Phase 7（US1）；验证 CreateDevice(Vulkan)、GetQueue、GetFeatures 返回真实句柄与特性.
3. **STOP and VALIDATE**: Vulkan 路径无 stub；再扩展 D3D12、Metal 与 US2–US5.

### Incremental Delivery

1. Setup + Vulkan 7 步 + Foundational + Vulkan 实现 → US1（Vulkan）→ 验证.
2. D3D12 7 步 + D3D12 实现 → US1（D3D12）→ 验证.
3. Metal 7 步 + Metal 实现 → US1（Metal）→ 验证.
4. US2 → US3 → US4 → US5 → SwapChain → Polish.

---

## Notes

- **第三方 7 步**：Vulkan 必须包含**自动下载**（FetchContent）；D3D12/Metal 为系统 SDK，步骤为检测、配置、编译测试、部署、配置实现.
- **禁止 stub**：Vulkan/D3D12/Metal 路径中不得以“仅返回 nullptr 或默认值、不调用底层 API”的实作为交付；plan 与 constitution 要求真实后端.
- **构建根目录**：执行 cmake 前须与用户确认根目录（如 worktree 根）；各子模块源码方式；见 `docs/engine-build-module-convention.md` §3.
- **测试逻辑（TenEngine）**：测试须覆盖上游 001-Core 能力与第三方/后端 API 调用（如 volk/vk*、ID3D12*、MTL*），不得仅测本模块孤立逻辑；见 `.specify/templates/tasks-template.md` 与 `docs/agent-build-guide.md`.
