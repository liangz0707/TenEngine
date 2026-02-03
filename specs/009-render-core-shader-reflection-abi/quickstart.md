# Quickstart: 009-RenderCore Shader Reflection 对接

**Branch**: `009-render-core-shader-reflection-abi` | **Date**: 2026-02-03  
**Audience**: 实现者与下游消费者；仅使用契约声明的类型与 API。

## 前置条件

- Core、RHI 已初始化（见 001-core-public-api、008-rhi-public-api）。
- 构建：C++17、CMake；依赖 008-RHI 源码引入（add_subdirectory）；008-RHI 拉入 001-Core。

## 最小使用示例（te::rendercore）

### 1. ResourceDesc：创建顶点/索引与纹理/缓冲描述

```cpp
#include <te/rendercore/api.hpp>
using namespace te::rendercore;

VertexAttribute attrs[] = { {0, VertexAttributeFormat::Float3, 0}, {1, VertexAttributeFormat::Float2, 12} };
VertexFormat vf = CreateVertexFormat({attrs, 2, 20});

TextureDescParams tp{1024, 768, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
TextureDesc tex = CreateTextureDesc(tp);

BufferDescParams bp{4096, BufferUsage::Uniform, 256};
BufferDesc buf = CreateBufferDesc(bp);
```

### 2. UniformLayout：手写或 010-Shader 反射产出

```cpp
UniformMember members[2] = {};
strncpy(members[0].name, "MVP", 63); members[0].type = UniformMemberType::Mat4; members[0].offset = 0; members[0].size = 64;
strncpy(members[1].name, "color", 63); members[1].type = UniformMemberType::Float4; members[1].offset = 64; members[1].size = 16;
UniformLayoutDesc desc{members, 2, 80};

IUniformLayout* layout = CreateUniformLayout(desc);
size_t mvpOff = layout->GetOffset("MVP");
size_t total = layout->GetTotalSize();
// 使用 layout 创建 UniformBuffer 或与 Shader 绑定
```

### 3. UniformBuffer：创建、更新、绑定

```cpp
te::rhi::IDevice* device = /* 从 008-RHI 获取 */;
IUniformBuffer* ub = CreateUniformBuffer(layout, device);
ub->SetCurrentFrameSlot(0);
ub->Update(data, size);
ub->Bind(cmdList, 0);
ReleaseUniformBuffer(ub);
ReleaseUniformLayout(layout);
```

### 4. PassProtocol：声明读/写与生命周期

```cpp
PassHandle pass{1};
ResourceHandle res{100};
DeclareRead(pass, res);
DeclareWrite(pass, res);

PassResourceDecl decl{pass, res, true, false, ResourceLifetime::Transient};
SetResourceLifetime(decl, ResourceLifetime::Persistent);
```

## 与 010-Shader 反射对接

010-Shader GetReflection 产出 `te::rendercore::UniformLayoutDesc` 时，按以下约定：

- 成员类型映射：SPIR-V 基础类型 → UniformMemberType（float→Float, vec4→Float4, mat4→Mat4 等）
- 偏移：按 std140 规则，或填 0 由 CreateUniformLayout 计算
- 命名：与 Shader 中 uniform 块成员名一致

## 构建与测试

- **构建**：`cmake -B build -DTENENGINE_RENDERCORE_BUILD_TESTS=ON`；008-RHI 须为同级目录或 `TENENGINE_RHI_DIR`
- **单元测试**：`build/tests/Release/test_render_core.exe`
- **契约测试**：`build/tests/Release/test_rhi_integration.exe`

## 参考

- 规约：`docs/module-specs/009-render-core.md`
- 契约：`specs/_contracts/009-rendercore-public-api.md`
- ABI：`specs/_contracts/009-rendercore-ABI.md`
- 全量 ABI（实现参考）：`contracts/009-rendercore-ABI-full.md`
