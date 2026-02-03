# Implementation Plan: 010-Shader Full Module Implementation

**Branch**: `010-shader-fullmodule-001` | **Date**: 2026-02-03 | **Spec**: [spec.md](./spec.md)  
**Input**: Feature specification from `specs/010-shader-fullmodule-001/spec.md`

**模块标识**：010-shader（从 worktree TenEngine-010-shader 推断）

## Summary

实现 010-Shader 模块完整能力：HLSL/GLSL 加载与编译、多后端（SPIR-V/DXIL/MSL）产出、宏与变体、预编译缓存、热重载。技术栈 C++17、CMake；参考 Unity、Unreal 的 Shader 模块与 API 构造。规约见 `docs/module-specs/010-shader.md`，契约见 `specs/_contracts/010-shader-public-api.md`。上游契约：001-core、008-rhi、009-rendercore。

## 实现范围（TenEngine：实现全量 ABI 内容）

**全量 ABI 内容**：见 `contracts/010-shader-ABI-full.md`。包括：
- **原始 ABI**：`specs/_contracts/010-shader-ABI.md` 中已声明的类型、接口、成员函数
- **新增**：工厂函数（Create/Destroy）、ReleaseHandle、LoadSourceFromMemory、SourceChangedCallback、factory.hpp
- **修改**：OnSourceChanged 增加 userData 参数

**实现时必须基于 `contracts/010-shader-ABI-full.md` 的全量内容**；禁止 stub 或空实现。第三方依赖须完整配置；与 008-RHI 共享 vulkan-headers 时按 `docs/third_party-integration-workflow.md` §7 处理。

## Technical Context

**Language/Version**: C++17  
**Primary Dependencies**: te_core, te_rhi, te_rendercore；glslang, spirv-cross, vulkan-headers, dxc, spirv-tools  
**Storage**: 文件（shader 源码、缓存）；内存（bytecode）  
**Testing**: 单元测试（编译、变体、缓存、热重载）；集成测试（与 RHI 对接）  
**Target Platform**: Windows (D3D11/D3D12)、Linux/macOS (Vulkan)、macOS/iOS (Metal)

**Project Type**: 引擎渲染模块  
**Performance Goals**: 简单 vertex shader 编译 <500ms；变体选择不阻塞渲染线程  
**Constraints**: Cache 单线程；首版不实现 Shader Graph

## 依赖引入方式（TenEngine 构建规约）

| 依赖模块 | 引入方式 | 说明 |
|----------|----------|------|
| 001-core | **源码** | TenEngineHelpers / tenengine_resolve_my_dependencies 引入 |
| 008-rhi | **源码** | 同上 |
| 009-rendercore | **源码** | 同上 |

### 第三方依赖（本 feature 涉及模块所需）

**共享依赖处理**：vulkan-headers 与 008-RHI 共享。引入前检查 `if(NOT TARGET Vulkan::Headers)`；若已存在（008-RHI 先拉取）则直接 `target_link_libraries` 使用，不再 FetchContent。FetchContent 内容名须为 `Vulkan-Headers`，GIT_TAG v1.3.280（与 008-RHI 一致）。详见 `docs/third_party-integration-workflow.md` §7。

| 第三方 ID | 引入方式 | 文档 | 说明 |
|-----------|----------|------|------|
| glslang | source | [glslang.md](../../docs/third_party/glslang.md) | GLSL/HLSL→SPIR-V |
| spirv-cross | source | [spirv-cross.md](../../docs/third_party/spirv-cross.md) | SPIR-V→MSL/HLSL |
| vulkan-headers | header-only | [vulkan-headers.md](../../docs/third_party/vulkan-headers.md) | 与 008-RHI 共享，复用 Vulkan::Headers |
| dxc | sdk | [dxc.md](../../docs/third_party/dxc.md) | HLSL→DXIL（D3D12） |
| spirv-tools | source | [spirv-tools.md](../../docs/third_party/spirv-tools.md) | glslang 依赖，通常随 glslang 拉取 |

**必须完整配置**：Task 阶段为每个第三方生成「版本选择、自动下载、配置、安装、编译测试、部署、配置实现」等步骤；禁止使用 stub 或空实现。

## Constitution Check

- [x] §I 模块边界：010-Shader 独立模块，与 Core/RHI/RenderCore 明确边界  
- [x] §II 现代图形 API：SPIR-V/DXIL/MSL 产出  
- [x] §III 数据驱动：Shader 为数据，宏/变体可配置  
- [x] §VI 契约优先：实现仅使用契约声明 API；全量 ABI 实现；构建引入真实子模块；禁止 stub

## Project Structure

### Documentation (this feature)

```text
specs/010-shader-fullmodule-001/
├── plan.md
├── research.md
├── data-model.md
├── quickstart.md
├── contracts/
│   └── 010-shader-ABI-full.md
└── tasks.md              # /speckit.tasks 产出
```

### Source Code

```text
include/te/shader/
├── types.hpp
├── factory.hpp
├── compiler.hpp
├── handle.hpp
├── cache.hpp
├── hot_reload.hpp
└── api.hpp

src/
├── types.cpp
├── factory.cpp
├── compiler_impl.cpp
├── handle_impl.cpp
├── cache_impl.cpp
├── hot_reload_impl.cpp
└── backends/             # glslang, spirv-cross, dxc 对接
    ├── glslang_backend.cpp
    ├── spirv_cross_backend.cpp
    └── dxc_backend.cpp

tests/
├── test_compile.cpp
├── test_variants.cpp
├── test_cache.cpp
└── test_hot_reload.cpp
```

**Structure Decision**: 单模块库；include 与契约 te/shader/ 一致；backends 按平台/宏编译。

## 契约更新（TenEngine，仅新增/修改部分 - 用于写回）

> 仅保存相对于 `specs/_contracts/010-shader-ABI.md` 的新增和修改。实现时使用 `contracts/010-shader-ABI-full.md` 全量内容。

| 操作 | 模块名 | 命名空间 | 符号 | 导出形式 | 头文件 | 说明 |
|------|--------|----------|------|----------|--------|------|
| 新增 | 010-Shader | te::shader | SourceChangedCallback | 类型别名 | te/shader/types.hpp | `using SourceChangedCallback = void (*)(char const* path, void* userData);` |
| 新增 | 010-Shader | te::shader | CreateShaderCompiler | 自由函数 | te/shader/factory.hpp | `IShaderCompiler* CreateShaderCompiler();` 失败返回 nullptr |
| 新增 | 010-Shader | te::shader | DestroyShaderCompiler | 自由函数 | te/shader/factory.hpp | `void DestroyShaderCompiler(IShaderCompiler* c);` |
| 新增 | 010-Shader | te::shader | CreateShaderCache | 自由函数 | te/shader/factory.hpp | `IShaderCache* CreateShaderCache();` 失败返回 nullptr |
| 新增 | 010-Shader | te::shader | DestroyShaderCache | 自由函数 | te/shader/factory.hpp | `void DestroyShaderCache(IShaderCache* c);` |
| 新增 | 010-Shader | te::shader | CreateShaderHotReload | 自由函数 | te/shader/factory.hpp | `IShaderHotReload* CreateShaderHotReload(IShaderCompiler* compiler, IShaderCache* cache);` 失败返回 nullptr |
| 新增 | 010-Shader | te::shader | DestroyShaderHotReload | 自由函数 | te/shader/factory.hpp | `void DestroyShaderHotReload(IShaderHotReload* h);` |
| 新增 | 010-Shader | te::shader | IShaderCompiler::LoadSourceFromMemory | 成员函数 | te/shader/compiler.hpp | `IShaderHandle* LoadSourceFromMemory(void const* data, size_t size, ShaderSourceFormat format) = 0;` |
| 新增 | 010-Shader | te::shader | IShaderCompiler::ReleaseHandle | 成员函数 | te/shader/compiler.hpp | `void ReleaseHandle(IShaderHandle* handle) = 0;` |
| 修改 | 010-Shader | te::shader | IShaderHotReload::OnSourceChanged | 成员函数 | te/shader/hot_reload.hpp | 签名增加 `void* userData = nullptr`：`void OnSourceChanged(char const* path, SourceChangedCallback callback, void* userData = nullptr) = 0;` |

## Complexity Tracking

无。
