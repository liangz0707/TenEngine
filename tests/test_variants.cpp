#include <te/shader/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <vector>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    assert(compiler && "CreateShaderCompiler failed");

    const char* glsl = R"(
        #version 450
        #ifdef USE_SKINNING
        layout(location=0) in vec4 weights;
        #endif
        void main() {
            gl_Position = vec4(0,0,0,1);
        }
    )";
    te::shader::IShaderHandle* handle = compiler->LoadSourceFromMemory(glsl, std::strlen(glsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handle && "LoadSourceFromMemory failed");

    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::SPIRV;

    std::vector<te::shader::VariantKey> keys;
    struct Enumerator : te::shader::IVariantEnumerator {
        std::vector<te::shader::VariantKey>* keys;
        void OnVariant(te::shader::VariantKey key) override { keys->push_back(key); }
    } enumerator;
    enumerator.keys = &keys;
    compiler->EnumerateVariants(handle, &enumerator);

    std::printf("EnumerateVariants: %zu variants\n", keys.size());
    assert(keys.size() >= 1 && "Expected at least one variant");

    bool ok = compiler->Precompile(handle, keys.data(), keys.size());
    if (!ok) {
        std::printf("Precompile failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handle);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }

    for (size_t i = 0; i < keys.size(); ++i) {
        handle->SelectVariant(keys[i]);
        size_t size = 0;
        void const* bytecode = compiler->GetBytecode(handle, &size);
        assert(bytecode && size > 0 && "GetBytecode failed for variant");
        std::printf("Variant %zu: bytecode %zu bytes (cache hit)\n", i, size);
    }

    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCompiler(compiler);
    std::printf("te_shader test_variants: all OK\n");
    return 0;
}
