# Quickstart: 009-render-core full

**Branch**: `009-render-core-full` | **Date**: 2026-01-29  
**Audience**: 实现者与下游消费者；仅使用契约声明的类型与 API。

## 前置条件

- Core、RHI 已初始化（见 001-core-public-api、008-rhi-public-api）。
- 构建：C++17、CMake；依赖 001-Core、008-RHI 源码引入（见 plan.md 依赖引入方式）。

## 最小使用示例（意图，非可编译代码）

### 1. ResourceDesc：创建顶点/索引与纹理/缓冲描述符

```text
VertexFormat vf = CreateVertexFormat({ attributes, stride });
IndexFormat idx = CreateIndexFormat(IndexFormatDesc::Index32);
TextureDesc texDesc = CreateTextureDesc({ width, height, format, usage });
BufferDesc bufDesc = CreateBufferDesc({ size, usage });
// 将 vf/idx/texDesc/bufDesc 传入 RHI 创建接口（008-RHI 契约）；不支持的格式/尺寸会在此处拒绝。
```

### 2. ShaderParams：定义 Uniform 布局并解析偏移

```text
UniformLayout layout = DefineLayout({ { "MVP", Matrix4 }, { "color", Vector4 } });
size_t off = GetOffset(layout, "MVP");
// 使用 layout 与 off 与 Shader 绑定、创建 Uniform 缓冲。
```

### 3. UniformBuffer：创建缓冲、更新、RingBuffer、绑定

```text
UniformBufferHandle ub = CreateUniformBuffer(layout);   // 根据 layout 创建 Uniform 缓冲
Update(ub, data, size);
RingBufferAdvance(ub);   // 或多帧时 AllocSlot；无槽位时 Block
Bind(ub, slot);          // 绑定到 RHI slot
```

### 4. PassProtocol：声明 Pass 读/写与生命周期

```text
PassResourceDecl readDecl  = DeclareRead(pass, resource);
PassResourceDecl writeDecl = DeclareWrite(pass, resource);
SetResourceLifetime(readDecl, ResourceLifetime::Transient);
// 声明交由 PipelineCore（019）消费，构建 Pass 图；同资源同 pass 读写由 PipelineCore 定义。
```

## 构建与测试

- 构建：在主工程或工作区根目录执行 CMake；009-render-core 作为子目录或 target 链接 001-Core、008-RHI。
- 单元测试：描述符与布局、GetOffset、RingBuffer 行为。
- 契约测试：VertexFormat/TextureDesc/BufferDesc 与 RHI 创建参数对接。
- 集成测试（可选）：Pass 声明与 PipelineCore 协议对齐。

## 参考

- 规约：`docs/module-specs/009-render-core.md`
- 契约：`specs/_contracts/009-rendercore-public-api.md`
- 上游：`specs/_contracts/001-core-public-api.md`、`specs/_contracts/008-rhi-public-api.md`
- 本 feature plan：`specs/009-render-core-full/plan.md`（含 API 雏形与契约更新）
