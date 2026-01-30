# Implementation Plan: 008-RHI 完整功能

**Branch**: `008-rhi-fullversion-001` | **Date**: 2026-01-29 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/008-rhi-fullversion-001/spec.md`。规约 `docs/module-specs/008-rhi.md`，契约 `specs/_contracts/008-rhi-public-api.md`，上游契约 `specs/_contracts/001-core-public-api.md`。本切片实现**完整模块范围**：Device、CommandList、Resources、PSO、Synchronization。

## Summary

实现 008-RHI **完整功能**：设备与队列、命令列表录制与提交、资源与视图、PSO 与 Shader 绑定、Fence/Semaphore 及资源屏障。技术栈 C++17、CMake；仅暴露/使用契约已声明的类型与 API。依赖 001-Core 源码引入。本 feature 产出可粘贴到契约的 **API 雏形** 见文末「契约更新」节。

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: 001-Core（内存、平台、日志）；Vulkan/D3D12/Metal 等图形 API 按平台/后端条件引入，见实现与构建配置。  
**Storage**: N/A（无业务持久化；PSO 缓存等由实现定义）  
**Testing**: CMake + CTest；`tenengine_add_module_test`，仅 link `te_rhi`。  
**Target Platform**: Windows / Linux / macOS（由 001-Core 平台抽象与后端条件编译决定）

## 依赖引入方式（TenEngine 构建规约：必须澄清）

| 依赖模块       | 引入方式   | 说明 |
|----------------|------------|------|
| **001-core**   | **源码**   | 通过 TenEngineHelpers 引入上游源码构建；RHI 使用 Alloc/Free、Log、平台检测等契约声明的 API。 |

**说明**：Vulkan SDK、D3D12、Metal 等图形 API 由本模块构建配置按后端可选引入；不写入上表。未列出之模块依赖按**源码**处理。

**Project Type**: 静态库（RHI 层），单一 CMake 工程。  
**Performance Goals**: 命令提交、资源创建与同步为引擎核心路径；具体帧预算与 draw call 上限由上层约束，本 plan 不设定数值。  
**Constraints**: 仅暴露/使用契约声明的类型与 API；多线程行为由实现定义并文档化（见 spec 澄清）。  
**Scale/Scope**: 本 feature 覆盖完整 008-RHI 模块范围。

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

| 原则 | 状态 | 说明 |
|------|------|------|
| I. Modular Renderer Architecture | 通过 | RHI 为独立、可测模块；边界清晰。 |
| II. Modern Graphics API First | 通过 | 抽象面向 Vulkan/D3D12/Metal；显式资源与管线管理。 |
| III. Data-Driven Pipeline | 通过 | PSO/Shader 数据驱动；与 RenderCore/Shader 对接。 |
| IV. Performance & Observability | 通过 | 使用 Core 日志；帧预算等由上层与实现约定。 |
| V. Versioning & ABI Stability | 通过 | 公开 API 按契约版本化；破坏性变更递增 MAJOR。 |
| VI. Module Boundaries & Contract-First | 通过 | 仅暴露 `008-rhi-public-api.md` 声明类型与 API；仅使用 `001-core-public-api.md` 已声明接口。 |

## Project Structure

### Documentation (this feature)

```text
specs/008-rhi-fullversion-001/
├── plan.md              # 本文件
├── research.md          # Phase 0
├── data-model.md        # Phase 1
├── quickstart.md        # Phase 1
├── contracts/           # Phase 1（可选：FR→API 映射）
└── tasks.md             # /speckit.tasks 产出
```

### Source Code (repository root)

```text
include/te/rhi/
├── device.hpp           # IDevice, CreateDevice, DestroyDevice, SelectBackend, GetSelectedBackend, GetQueue, GetFeatures
├── queue.hpp            # IQueue
├── command_list.hpp     # ICommandList, Begin, End, Draw, Dispatch, Copy, ResourceBarrier, Submit
├── resources.hpp        # IBuffer, ITexture, ISampler, Create*, CreateView, Destroy
├── pso.hpp              # IPSO, CreateGraphicsPSO, CreateComputePSO, SetShader, Cache
├── sync.hpp             # IFence, ISemaphore, CreateFence, CreateSemaphore, Wait, Signal
└── types.hpp            # Backend, QueueType, DeviceFeatures, 描述符等

src/
├── device/              # Device、Queue、GetFeatures 实现
├── command_list/        # 命令缓冲录制与提交
├── resources/           # Buffer、Texture、Sampler、View
├── pso/                 # PSO 创建、绑定、缓存
└── sync/                # IFence、ISemaphore、屏障

tests/
├── device_*             # 设备、队列、特性、后端
├── command_list_*       # 录制、提交
├── resources_*          # 资源与视图
├── pso_*                # PSO 创建与绑定
└── sync_*               # 同步
```

**Structure Decision**: 单库 `te_rhi`，公共 API 置于 `include/te/rhi/`；实现按 Device、CommandList、Resources、PSO、Sync 分子目录。测试通过 `tenengine_add_module_test` 仅 link `te_rhi`。

## Complexity Tracking

> 无 Constitution 违规需豁免。

| Violation | Why Needed | Simpler Alternative Rejected Because |
|-----------|------------|-------------------------------------|
| —         | —          | —                                   |

## Phase 0: Outline & Research

- **Unknowns resolved**: CreateDevice 与窗口/上下文（不涉及；窗口/上下文仅用于 Present/XR）；SelectBackend 与 CreateDevice（两路均支持）；GetFeatures 结构（plan 定义最小集）；后端不可用（仅失败，不自动回退）；线程安全（不要求默认并发安全，由实现定义并文档化）。
- **Research output**: 见 `research.md`。结论：Device/Queue/GetFeatures 与 device 切片 plan 一致；CommandList 由 IDevice 分配，Begin/End 录制，Submit 提交到 IQueue；Resources/PSO/Sync 依契约能力 3–5 设计，描述符与句柄见 data-model。

## Phase 1: Design & Contracts

- **Data model**: 见 `data-model.md`。实体：IDevice、IQueue、ICommandList、IBuffer、ITexture、ISampler、IPSO、Fence、Semaphore、资源视图句柄及各类描述符。
- **API contracts**: 本 feature 实现的对外 API 即契约完整能力；具体签名与类型见文末「契约更新」。`contracts/` 可选保留 FR→API 映射。
- **Quickstart**: 见 `quickstart.md`（CMake 配置、依赖 001-Core、构建、测试、最小示例）。

---

## 契约更新（API 雏形）

以下为 **本 feature 对外暴露** 的类型与函数签名，可直接粘贴到 `specs/_contracts/008-rhi-public-api.md` 的 **「API 雏形」** 小节。本 feature 实现完整模块范围（Device、CommandList、Resources、PSO、Synchronization）。

```markdown
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
```

上述内容即本 feature 的 **契约更新**；同步到 `T0-contracts` 分支上 `specs/_contracts/008-rhi-public-api.md` 的「API 雏形」后，实现与下游均以之为准。
