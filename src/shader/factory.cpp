#include <te/shader/factory.hpp>
#include <te/shader/detail/compiler_impl.hpp>
#include <te/shader/detail/cache_impl.hpp>
#include <te/shader/detail/hot_reload_impl.hpp>
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
#include <te/core/alloc.h>
#endif
#include <new>

namespace te::shader {

#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE

IShaderCompiler* CreateShaderCompiler() {
    void* p = te::core::Alloc(sizeof(ShaderCompilerImpl), alignof(ShaderCompilerImpl));
    if (!p) return nullptr;
    return new (p) ShaderCompilerImpl();
}

void DestroyShaderCompiler(IShaderCompiler* c) {
    if (!c) return;
    static_cast<ShaderCompilerImpl*>(c)->~ShaderCompilerImpl();
    te::core::Free(c);
}

IShaderCache* CreateShaderCache() {
    void* p = te::core::Alloc(sizeof(ShaderCacheImpl), alignof(ShaderCacheImpl));
    if (!p) return nullptr;
    return new (p) ShaderCacheImpl();
}

void DestroyShaderCache(IShaderCache* c) {
    if (!c) return;
    static_cast<ShaderCacheImpl*>(c)->~ShaderCacheImpl();
    te::core::Free(c);
}

IShaderHotReload* CreateShaderHotReload(IShaderCompiler* compiler, IShaderCache* cache) {
    void* p = te::core::Alloc(sizeof(ShaderHotReloadImpl), alignof(ShaderHotReloadImpl));
    if (!p) return nullptr;
    return new (p) ShaderHotReloadImpl(compiler, cache);
}

void DestroyShaderHotReload(IShaderHotReload* h) {
    if (!h) return;
    static_cast<ShaderHotReloadImpl*>(h)->~ShaderHotReloadImpl();
    te::core::Free(h);
}

#else

IShaderCompiler* CreateShaderCompiler() {
    return new ShaderCompilerImpl();
}

void DestroyShaderCompiler(IShaderCompiler* c) {
    delete c;
}

IShaderCache* CreateShaderCache() {
    return new ShaderCacheImpl();
}

void DestroyShaderCache(IShaderCache* c) {
    delete c;
}

IShaderHotReload* CreateShaderHotReload(IShaderCompiler* compiler, IShaderCache* cache) {
    return new ShaderHotReloadImpl(compiler, cache);
}

void DestroyShaderHotReload(IShaderHotReload* h) {
    delete h;
}

#endif

}  // namespace te::shader
