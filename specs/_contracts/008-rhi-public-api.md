# 契约：008-RHI 渲染抽象层对外 API（RCI）

## 适用模块

- **实现方**：**008-RHI**（Render Hardware Interface，T0 图形 API 抽象层）
- **对应规格**：`docs/module-specs/008-rhi.md`
- **角色**：向上层提供统一渲染接口，向下对接 **Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等图形接口（及 **GLSL**、**HLSL/DXIL**、**MSL** 等 Shader 接口，与 010-Shader 对接）；RCI 与 RHI 同义，本契约即 RHI 对外 API。**引擎统一使用 TenEngine 命名空间**，本模块 API 位于 `TenEngine::rhi`。**可以通过宏来判断执行哪一段代码**（如 TE_RHI_VULKAN、TE_RHI_METAL、TE_RHI_D3D12），编译时选择后端相关实现路径。

## 消费者（T0 下游）

- 009-RenderCore（渲染类型与 Pass 协议、资源描述）
- 010-Shader（提交 Shader 字节码、PSO 绑定）
- 019-PipelineCore（命令列表、资源、屏障、提交约定）
- 020-Pipeline（命令缓冲生成与提交、SwapChain/XR）
- 024-Editor（视口渲染）
- 027-XR（提交到 XR 交换链）

## 第三方依赖

- **第三方**：volk、vulkan-headers（Vulkan 后端）；d3d11、d3d12（Windows 后端）；metal（Apple 后端）。详见 `docs/third_party/` 对应 md，按平台与编译选项选择。

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）
- **ABI 定义**：`specs/_contracts/008-rhi-ABI.md`

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| IDevice | 图形设备抽象；创建队列、资源、PSO | 由 RHI 管理，创建后直至 DestroyDevice |
| IQueue | 队列句柄；由 IDevice::GetQueue 返回 | 非拥有，生命周期与 IDevice 一致 |
| ICommandList | 命令缓冲录制；Draw/Dispatch/Copy、ResourceBarrier、Submit | 单次录制周期内有效 |
| IBuffer / ITexture | 缓冲、纹理；可带视图（SRV/UAV/RTV/DSV 等概念） | 创建后直至显式 Destroy |
| ISampler | 采样器 | 创建后直至显式销毁 |
| IPSO | 管线状态对象（图形/计算）；与 Shader 字节码绑定 | 创建后直至显式销毁，可缓存 |
| IFence / ISemaphore | 同步对象；多队列、跨帧同步 | 按实现约定 |
| 资源视图句柄（ViewHandle） | 描述符或视图 ID，用于绑定到 PSO | 与资源生命周期一致 |

下游仅通过上述抽象类型与句柄访问，不暴露具体后端（Vulkan/Metal/D3D12）类型。平台与后端**可通过宏选择执行哪一段代码**（001-Core 平台宏、本模块 TE_RHI_* 后端宏）。

## 渲染资源显式控制位置

**准备/创建/更新 GPU 资源**（**CreateBuffer**、**CreateTexture**、**CreateSampler**、**CreateView**、**CreateGraphicsPSO**、**CreateComputePSO** 等）由本模块提供；**提交到实际 GPU Command** 通过 **Submit(ICommandList*, IQueue*)** 完成。创建逻辑渲染资源（CreateRenderItem）、收集逻辑 CommandBuffer（CollectCommandBuffer）、准备渲染资源（PrepareRenderMaterial、PrepareRenderMesh）见 019-PipelineCore/020-Pipeline。

## 能力列表（提供方保证）

1. **设备与队列**：创建设备、获取队列、特性检测、后端选择（**Vulkan**、**Metal（MTL）**、**D3D12/DXIL** 等）；多后端统一接口；**可通过宏判断执行哪一段代码**（编译时选择后端）。
2. **命令列表**：录制绘制/计算/拷贝命令、资源屏障、提交到队列；Begin/End、Draw、Dispatch、Copy、ResourceBarrier、Submit 语义明确。
3. **资源管理**：CreateBuffer、CreateTexture、CreateSampler、CreateView；DestroyBuffer、DestroyTexture、DestroySampler；内存管理与生命周期明确；失败时有明确报告。
4. **PSO**：CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO；与 RenderCore/Shader 模块对接。
5. **同步**：CreateFence、CreateSemaphore、Wait、Signal、DestroyFence、DestroySemaphore；资源屏障在 ICommandList::ResourceBarrier；提交顺序与等待语义明确。
6. **错误与恢复**：设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃。
7. **线程安全**：RHI 接口在多线程访问下的行为由实现定义并文档化。

## 调用顺序与约束

- 上层须先完成 RHI 初始化（及所需窗口/上下文，可能依赖 Core 平台抽象或 003-Application），再创建资源与提交命令。
- 资源销毁顺序不得违反底层 API 要求（如先释放依赖该资源的命令或 PSO）。
- Pipeline（020）产出的命令缓冲通过本契约约定的提交接口（Submit）交给 RHI；具体数据结构与提交时机见契约 `pipeline-to-rci.md`。

## 与流水线的边界

- 渲染管线（020-Pipeline）按 PipelineCore（019）协议构建 Pass 图与命令缓冲，最终通过本契约的**提交接口 Submit(ICommandList*, IQueue*)** 交给 RHI 执行；命令缓冲格式与资源状态约定见 `pipeline-to-rci.md`。

## API 雏形（简化声明，命名空间 TenEngine::rhi）

与 `docs/module-specs/008-rhi.md` §5.2 及 `specs/_contracts/008-rhi-ABI.md` 对齐。

### 1. Device 与后端

- 枚举 `Backend`: `Vulkan`, `D3D12`, `Metal`。
- `void SelectBackend(Backend b);` — 设置默认后端；`CreateDevice()` 无参时使用。
- `Backend GetSelectedBackend();`
- `struct IDevice;` — 图形设备抽象；创建后直至 `DestroyDevice`，不暴露后端类型。
- `IDevice* CreateDevice(Backend backend);` — 成功返回有效指针，失败 `nullptr`。不依赖窗口/上下文。
- `IDevice* CreateDevice();` — 使用 `GetSelectedBackend()`。
- `void DestroyDevice(IDevice* device);` — `nullptr` 为 no-op。

### 2. 队列

- 枚举 `QueueType`: `Graphics`, `Compute`, `Copy`。
- `struct IQueue;` — 队列句柄；由 `IDevice::GetQueue` 返回。
- `IQueue* IDevice::GetQueue(QueueType type, uint32_t index);` — 无效或越界返回 `nullptr`。非拥有，生命周期与 `IDevice` 一致。

### 3. 特性查询

- `struct DeviceFeatures` — 可读、可验证；至少 `maxTextureDimension2D`、`maxTextureDimension3D`。
- `DeviceFeatures const& IDevice::GetFeatures() const;`

### 4. 命令列表

- `struct ICommandList;` — 命令缓冲；单次录制周期内有效，由 IDevice 分配。
- `ICommandList* IDevice::CreateCommandList();` — 失败返回 `nullptr`。
- `void IDevice::DestroyCommandList(ICommandList* cmd);` — `nullptr` 为 no-op。
- `void Begin(ICommandList* cmd);` / `void End(ICommandList* cmd);`
- `void ICommandList::Draw(uint32_t, uint32_t=1, uint32_t=0, uint32_t=0);` / `Dispatch(uint32_t, uint32_t=1, uint32_t=1);` / `Copy(void const*, void*, size_t);` / `ResourceBarrier();`
- `void Submit(ICommandList* cmd, IQueue* queue);`

### 5. 资源与视图

- `struct IBuffer;` / `struct ITexture;` / `struct ISampler;` — 创建后直至显式销毁。
- `IBuffer* IDevice::CreateBuffer(BufferDesc const& desc);` / `CreateTexture` / `CreateSampler`
- `ViewHandle IDevice::CreateView(ViewDesc const& desc);`
- `void IDevice::DestroyBuffer(IBuffer*);` / `DestroyTexture(...)` / `DestroySampler(...)`

### 6. PSO

- `struct IPSO;` — 管线状态对象（图形/计算）；与 Shader 字节码绑定；可缓存，创建后直至显式销毁。
- `IPSO* IDevice::CreateGraphicsPSO(GraphicsPSODesc const& desc);` / `CreateComputePSO(ComputePSODesc const& desc);`
- `void IDevice::SetShader(IPSO* pso, void const* data, size_t size);` / `void Cache(IPSO* pso);` / `void DestroyPSO(IPSO* pso);`

### 7. 同步

- `struct IFence;` / `struct ISemaphore;` — 多队列、跨帧同步。
- `IFence* IDevice::CreateFence();` / `ISemaphore* IDevice::CreateSemaphore();`
- `void Wait(IFence* f);` / `void Signal(IFence* f);`
- `void IDevice::DestroyFence(IFence*);` / `DestroySemaphore(ISemaphore*);`

### 8. 错误与约束

- 后端不可用时 `CreateDevice` 返回 `nullptr`，不自动回退；回退与重试由上层负责。
- 多线程行为由实现定义并文档化；本契约不要求默认并发安全。
- 实现仅使用 `001-core-public-api.md` 已声明的类型与 API。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 002-rendering-rci-interface spec 提炼，供多 Agent 引用 |
| T0 更新 | 对齐 T0 架构 008-RHI：实现方改为 008-RHI，消费者改为 T0 模块列表；类型与能力与 docs/module-specs/008-rhi.md 一致 |
| 2026-01-29 | 契约与模块规约 §5.2 对齐：统一使用 TenEngine::rhi、CreateDevice/GetQueue/CreateBuffer/CreateTexture 等 API；能力 3 改为 CreateBuffer/CreateTexture/CreateSampler/CreateView；类型表补充 IQueue、IFence、ISemaphore、ViewHandle；新增 API 雏形 |
| 2026-01-29 | contract(008-rhi): sync from plan 008-rhi-fullmodule-001；ABI 表新增 Phase 1 实现小节，与 public-api API 雏形及 docs/module-specs/008-rhi.md §5.2 对齐 |
