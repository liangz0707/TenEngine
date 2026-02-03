# Quickstart: 010-Shader Module

## 构建

```bash
# 从 worktree 根目录
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

**依赖**：TenEngine-001-core、TenEngine-008-rhi、TenEngine-009-render-core 须在 `TENENGINE_ROOT` 或同级目录可用；由 `tenengine_resolve_my_dependencies` 自动引入。

**第三方**：glslang、spirv-cross、vulkan-headers、dxc（D3D12）、spirv-tools。见 plan.md「第三方依赖」；vulkan-headers 与 008-RHI 共享，使用 `if(NOT TARGET Vulkan::Headers)` 复用。

## 基本用法

```cpp
#include <te/shader/api.hpp>

// 1. 创建编译器与缓存
te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
te::shader::IShaderCache* cache = te::shader::CreateShaderCache();

// 2. 加载并编译
te::shader::CompileOptions opts{};
opts.targetBackend = te::shader::BackendType::SPIRV;
te::shader::IShaderHandle* handle = compiler->LoadSource("shaders/main.vert", te::shader::ShaderSourceFormat::GLSL);
if (handle) {
    if (compiler->Compile(handle, opts)) {
        size_t size;
        void const* bytecode = compiler->GetBytecode(handle, &size);
        // 提交 bytecode 给 RHI 创建 PSO
    } else {
        char const* err = compiler->GetLastError();
    }
    compiler->ReleaseHandle(handle);
}

// 3. 释放
te::shader::DestroyShaderCache(cache);
te::shader::DestroyShaderCompiler(compiler);
```

## 热重载

```cpp
te::shader::IShaderHotReload* hotReload = te::shader::CreateShaderHotReload(compiler, cache);
hotReload->OnSourceChanged("shaders/main.vert", [](char const* path, void* ud) {
    // 文件变更时回调；调用 ReloadShader 并 NotifyShaderUpdated
}, nullptr);
// ...
te::shader::DestroyShaderHotReload(hotReload);
```
