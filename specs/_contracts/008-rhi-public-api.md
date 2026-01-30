# 契约：008-RHI 渲染抽象层对外 API（RCI）

## 适用模块

- **实现方**：**008-RHI**（Render Hardware Interface，T0 图形 API 抽象层）
- **对应规格**：`docs/module-specs/008-rhi.md`
- **角色**：向上层提供统一渲染接口，向下对接 Vulkan/D3D12/Metal 等图形 API；RCI 与 RHI 同义，本契约即 RHI 对外 API。

## 消费者（T0 下游）

- 009-RenderCore（渲染类型与 Pass 协议、资源描述）
- 010-Shader（提交 Shader 字节码、PSO 绑定）
- 019-PipelineCore（命令列表、资源、屏障、提交约定）
- 020-Pipeline（命令缓冲生成与提交、SwapChain/XR）
- 024-Editor（视口渲染）
- 027-XR（提交到 XR 交换链）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| IDevice | 图形设备抽象；创建队列、资源、PSO | 由 RHI 管理，创建后直至销毁 |
| ICommandList | 命令缓冲录制；Draw/Dispatch/Copy、ResourceBarrier、Submit | 单次录制周期内有效 |
| IBuffer / ITexture | 缓冲、纹理；可带视图（SRV/UAV/RTV/DSV 等概念） | 创建后直至显式销毁 |
| ISampler | 采样器 | 创建后直至显式销毁 |
| IPSO | 管线状态对象（图形/计算）；与 Shader 字节码绑定 | 创建后直至显式销毁，可缓存 |
| Fence / Semaphore | 同步对象；多队列、跨帧同步 | 按实现约定 |
| 资源视图句柄 | 描述符或视图 ID，用于绑定到 PSO | 与资源生命周期一致 |

下游仅通过上述抽象类型与句柄访问，不暴露具体后端（Vulkan/D3D12/Metal）类型。

## 能力列表（提供方保证）

1. **设备与队列**：创建设备、获取队列、特性检测、后端选择（Vulkan/D3D12/Metal）；多后端统一接口。
2. **命令列表**：录制绘制/计算/拷贝命令、资源屏障、提交到队列；Begin/End、Draw、Dispatch、Copy、ResourceBarrier、Submit 语义明确。
3. **资源管理**：创建 Buffer、Texture、Sampler、视图；内存管理与生命周期明确；失败时有明确报告。
4. **PSO**：创建图形/计算管线状态对象、与 Shader 字节码/模块绑定、可选缓存与编译；与 RenderCore/Shader 模块对接。
5. **同步**：Fence、Semaphore、资源屏障、多队列同步；提交顺序与等待语义明确。
6. **错误与恢复**：设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃。
7. **线程安全**：RHI 接口在多线程访问下的行为由实现定义并文档化。

## 调用顺序与约束

- 上层须先完成 RHI 初始化（及所需窗口/上下文，可能依赖 Core 平台抽象或 003-Application），再创建资源与提交命令。
- 资源销毁顺序不得违反底层 API 要求（如先释放依赖该资源的命令或 PSO）。
- Pipeline（020）产出的命令缓冲通过本契约约定的提交接口交给 RHI；具体数据结构与提交时机见契约 `pipeline-to-rci.md`。

## 与流水线的边界

- 渲染管线（020-Pipeline）按 PipelineCore（019）协议构建 Pass 图与命令缓冲，最终通过本契约的**提交接口**交给 RHI 执行；命令缓冲格式与资源状态约定见 `pipeline-to-rci.md`。

## API 雏形（简化声明）

### 本 feature（008-rhi-fullversion-001）产出

#### 1. Device 与后端

- 枚举 `Backend`: `Vulkan`, `D3D12`, `Metal`。
- `void SelectBackend(Backend b);` — 设置默认后端；`CreateDevice()` 无参时使用。
- `Backend GetSelectedBackend();`

- `struct IDevice;` — 图形设备抽象；创建后直至 `DestroyDevice`，不暴露后端类型。
- `IDevice* CreateDevice(Backend backend);` — 成功返回有效指针，失败 `nullptr`。不依赖窗口/上下文。
- `IDevice* CreateDevice();` — 使用 `GetSelectedBackend()`。
- `void DestroyDevice(IDevice* device);` — `nullptr` 为 no-op。

#### 2. 队列

- 枚举 `QueueType`: `Graphics`, `Compute`, `Copy`。
- `struct IQueue;` — 队列句柄；由 `IDevice::GetQueue` 返回。
- `IQueue* IDevice::GetQueue(QueueType type, uint32_t index);` — 无效或越界返回 `nullptr`。非拥有，生命周期与 `IDevice` 一致。

#### 3. 特性查询

- `struct DeviceFeatures` — 可读、可验证；至少 `maxTextureDimension2D`、`maxTextureDimension3D` 等，具体字段由实现定义。
- `DeviceFeatures const& IDevice::GetFeatures() const;`

#### 4. 命令列表

- `struct ICommandList;` — 命令缓冲；单次录制周期内有效，由 IDevice 分配。
- `ICommandList* IDevice::CreateCommandList();` — 失败返回 `nullptr`。调用方在 Submit 后不再使用，或按实现约定归还/复用。
- `void IDevice::DestroyCommandList(ICommandList* cmd);` — `nullptr` 为 no-op。（若为池化复用，以实现为准。）

- `void Begin(ICommandList* cmd);` / `void End(ICommandList* cmd);` — 录制开始/结束。
- `void ICommandList::Draw(...);` / `void ICommandList::Dispatch(...);` / `void ICommandList::Copy(...);` — 具体签名见实现；语义与契约能力 2 一致。
- `void ICommandList::ResourceBarrier(...);` — 资源屏障；参数见实现。
- `void Submit(ICommandList* cmd, IQueue* queue);` — 将录制好的命令缓冲提交到队列执行。Pipeline 产出的抽象命令缓冲通过此接口或等价提交接口交给 RHI；格式见 `pipeline-to-rci.md`。

#### 5. 资源与视图

- `struct IBuffer;` / `struct ITexture;` / `struct ISampler;` — 创建后直至显式销毁。
- `IBuffer* IDevice::CreateBuffer(BufferDesc const& desc);` — 失败返回 `nullptr`。描述符字段见实现。
- `ITexture* IDevice::CreateTexture(TextureDesc const& desc);`
- `ISampler* IDevice::CreateSampler(SamplerDesc const& desc);`
- `ViewHandle IDevice::CreateView(ViewDesc const& desc);` — 资源视图句柄（SRV/UAV/RTV/DSV 等概念）；与资源生命周期一致。具体类型与描述符见实现。
- `void IDevice::DestroyBuffer(IBuffer*);` / `DestroyTexture(...)` / `DestroySampler(...)`；销毁前须释放依赖该资源的命令与 PSO。

#### 6. PSO

- `struct IPSO;` — 管线状态对象（图形/计算）；与 Shader 字节码绑定；可缓存，创建后直至显式销毁。
- `IPSO* IDevice::CreateGraphicsPSO(GraphicsPSODesc const& desc);` — 描述符含 Shader 字节码或模块引用等，见实现；与 RenderCore/Shader 对接。
- `IPSO* IDevice::CreateComputePSO(ComputePSODesc const& desc);`
- `void IDevice::SetShader(IPSO* pso, ...);` / `void Cache(IPSO* pso);` — 语义与契约能力 4 一致；具体签名见实现。
- `void IDevice::DestroyPSO(IPSO* pso);`

#### 7. 同步

- `struct IFence;` / `struct ISemaphore;` — 多队列、跨帧同步；按实现约定生命周期。
- `IFence* IDevice::CreateFence();` / `ISemaphore* IDevice::CreateSemaphore();`
- `void Wait(IFence* f);` / `void Signal(IFence* f);` — 或由 IQueue 参与，具体见实现。Semaphore 同理。
- `void IDevice::DestroyFence(IFence*);` / `DestroySemaphore(ISemaphore*);`

#### 8. 错误与约束

- 后端不可用时 `CreateDevice` 返回 `nullptr`，不自动回退；回退与重试由上层负责。
- 多线程行为由实现定义并文档化；本 feature 不要求默认并发安全。
- 实现仅使用 `001-core-public-api.md` 已声明的类型与 API。设备丢失或运行时错误可上报；支持回退或重建，不导致引擎崩溃。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 002-rendering-rci-interface spec 提炼，供多 Agent 引用 |
| T0 更新 | 对齐 T0 架构 008-RHI：实现方改为 008-RHI，消费者改为 T0 模块列表；类型与能力与 docs/module-specs/008-rhi.md 一致 |
| 2026-01-29 | API 雏形由 plan 008-rhi-fullversion-001 同步 |
