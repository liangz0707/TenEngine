#include <te/shader/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    assert(compiler && "CreateShaderCompiler failed");

    char const* hlsl = R"(
        float4 main(float4 pos : POSITION) : SV_POSITION { return pos; }
    )";
    te::shader::IShaderHandle* handle = compiler->LoadSourceFromMemory(hlsl, std::strlen(hlsl), te::shader::ShaderSourceFormat::HLSL);
    assert(handle && "LoadSourceFromMemory failed");

    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::SPIRV;
    opts.stage = te::shader::ShaderStage::Vertex;
    opts.entryPoint[0] = 'm'; opts.entryPoint[1] = 'a'; opts.entryPoint[2] = 'i'; opts.entryPoint[3] = 'n'; opts.entryPoint[4] = '\0';
    bool ok = compiler->Compile(handle, opts);
    if (!ok) {
        std::printf("HLSL Compile failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handle);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }

    size_t size = 0;
    void const* bytecode = compiler->GetBytecode(handle, &size);
    assert(bytecode && size > 0 && "GetBytecode failed");

    std::printf("te_shader test_hlsl_compile: OK (SPIR-V %zu bytes)\n", size);

    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCompiler(compiler);
    std::printf("te_shader test_hlsl_compile: all OK\n");
    return 0;
}
