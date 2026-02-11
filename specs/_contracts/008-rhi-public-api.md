# 契约：008-RHI 模块对外 API

## 适用模块

- **实现方**：008-RHI（L1；图形 API 抽象：设备、命令列表、资源、PSO、同步；多后端 Vulkan/D3D12/Metal；命名空间 `te::rhi`，后端宏 TE_RHI_*）
- **对应规格**：`docs/module-specs/008-rhi.md`
- **依赖**：001-Core

## 消费者

- 009-RenderCore、010-Shader、012-Mesh、019-PipelineCore、020-Pipeline、024-Editor、028-Texture

## 第三方依赖

- volk、vulkan-headers（Vulkan）；d3d11、d3d12（Windows）；metal（Apple）。按平台与编译选项选择；详见 `docs/third_party/` 对应文档。

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| IDevice | 图形设备抽象；创建队列、资源、PSO；CreateDevice、DestroyDevice | 创建后直至 DestroyDevice |
| IQueue | 队列句柄；IDevice::GetQueue(QueueType, index)；QueueType: Graphics, Compute, Copy | 非拥有，与 IDevice 一致 |
| ICommandList | 命令缓冲；Begin/End、Draw、DrawIndexed、Dispatch、Copy、ResourceBarrier、SetViewport、SetScissor、BeginRenderPass/EndRenderPass、Submit | 单次录制周期内有效 |
| IBuffer / ITexture | 缓冲、纹理；CreateBuffer、CreateTexture、CreateView、Destroy | 创建后直至显式 Destroy |
| ISampler | 采样器；CreateSampler、DestroySampler | 创建后直至显式销毁 |
| IPSO | 管线状态对象（图形/计算）；CreateGraphicsPSO、CreateComputePSO、SetShader、Cache、DestroyPSO | 创建后直至显式销毁，可缓存 |
| IFence / ISemaphore | 同步对象；CreateFence、CreateSemaphore、Wait、Signal、Destroy | 按实现约定 |
| IDescriptorSetLayout / IDescriptorSet | 描述符集布局与描述符集；CreateDescriptorSetLayout、AllocateDescriptorSet、UpdateDescriptorSet、BindDescriptorSet | 创建后直至显式销毁 |
| ViewHandle | 资源视图句柄，用于绑定到 PSO | 与资源生命周期一致 |
| Backend / DeviceFeatures / DeviceLimits | 后端枚举、特性与限制查询；GetFeatures、GetLimits | 只读 |

不暴露具体后端类型；012/028 在 EnsureDeviceResources 时调用本模块 CreateBuffer/CreateTexture。与 020 的提交约定见 `pipeline-to-rci.md`。

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 设备与队列 | CreateDevice(Backend)、DestroyDevice、GetQueue；SelectBackend、GetSelectedBackend；GetFeatures、GetLimits；多后端统一接口 |
| 2 | 命令列表 | CreateCommandList、DestroyCommandList；Begin、End；Draw、DrawIndexed、Dispatch、Copy、ResourceBarrier；SetViewport、SetScissor；SetUniformBuffer、SetVertexBuffer、SetIndexBuffer、SetGraphicsPSO、BindDescriptorSet；BeginRenderPass、EndRenderPass；BeginOcclusionQuery、EndOcclusionQuery；Submit(cmd, queue) 及 Fence/Semaphore 重载 |
| 3 | 资源管理 | CreateBuffer、CreateTexture、CreateSampler、CreateView；Destroy；内存与生命周期明确；失败有明确报告 |
| 4 | PSO | CreateGraphicsPSO(desc)、CreateGraphicsPSO(desc, layout)（与 descriptor layout 耦合）、CreateComputePSO、SetShader、Cache、DestroyPSO；与 RenderCore/Shader 对接 |
| 5 | 同步 | CreateFence、CreateSemaphore、Wait、Signal、Destroy；资源屏障在 ICommandList::ResourceBarrier |
| 6 | 错误与恢复 | 设备丢失或运行时错误可上报；支持回退或重建 |
| 7 | 线程安全 | 多线程行为由实现定义并文档化 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须先完成 RHI 初始化后再创建资源与提交命令。资源销毁顺序须符合底层 API 要求。020 产出的命令缓冲经 Submit 交 RHI；格式与时机见 `pipeline-to-rci.md`。

## TODO 列表

（以下任务来自 `docs/asset/` 资源管理/加载/存储设计。）

- [ ] **DResource 创建**：提供 CreateBuffer、CreateTexture、CreateSampler、CreateGraphicsPSO 等接口，供 028/011/012 在 EnsureDeviceResources 时调用；不暴露具体后端类型；生命周期与 028/011/012 协调。
- [ ] **数据与接口**（原 ABI 数据相关 TODO）：BufferDesc（size、usage=Vertex/Index/Uniform）、TextureDesc（width、height、format、mipLevels）；IDevice::CreateTexture(TextureDesc)、CreateBuffer(BufferDesc)、UpdateBuffer(buffer, offset, data, size)；ICommandList::SetUniformBuffer(slot, buffer, offset)；设备/资源内部分配调用 001 Alloc/Free。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| （初始） | 从 002-rendering-rci-interface spec 提炼 |
| T0 更新 | 对齐 T0 架构；消费者按依赖图 |
| 2026-02-05 | 统一目录；能力列表用表格；去除 ABI 反向引用 |
| 2026-02-10 | 能力 2 命令列表：补充 SetVertexBuffer、SetIndexBuffer、SetGraphicsPSO、BeginOcclusionQuery、EndOcclusionQuery |
| 2026-02-10 | 能力 2/4：BindDescriptorSet；CreateGraphicsPSO(desc, layout)；描述符集 API 已实现 |
