#include <te/shader/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    te::shader::IShaderCache* cache = te::shader::CreateShaderCache();
    assert(compiler && cache && "Create failed");

    compiler->SetCache(cache);

    const char* glsl = R"(
        #version 450
        void main() { gl_Position = vec4(0,0,0,1); }
    )";
    te::shader::IShaderHandle* handle = compiler->LoadSourceFromMemory(glsl, std::strlen(glsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handle && "LoadSourceFromMemory failed");

    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::SPIRV;
    bool ok = compiler->Compile(handle, opts);
    assert(ok && "Compile failed");

    size_t size1 = 0;
    void const* bytecode1 = compiler->GetBytecode(handle, &size1);
    assert(bytecode1 && size1 > 0 && "GetBytecode failed");

    char const* cachePath = "test_shader_cache.bin";
    ok = cache->SaveCache(cachePath);
    assert(ok && "SaveCache failed");

    cache->Invalidate(handle);
    size_t size2 = 0;
    void const* bytecode2 = compiler->GetBytecode(handle, &size2);
    assert(!bytecode2 && "GetBytecode should be empty after Invalidate");

    ok = cache->LoadCache(cachePath);
    assert(ok && "LoadCache failed");

    ok = compiler->Compile(handle, opts);
    assert(ok && "Re-Compile (from cache) failed");

    size_t size3 = 0;
    void const* bytecode3 = compiler->GetBytecode(handle, &size3);
    assert(bytecode3 && size3 > 0 && "GetBytecode after LoadCache failed");
    assert(size3 == size1 && "Bytecode size mismatch after restore");

    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCache(cache);
    te::shader::DestroyShaderCompiler(compiler);

    std::printf("te_shader test_cache: all OK\n");
    return 0;
}
