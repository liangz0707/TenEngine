#include <te/shader/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    assert(compiler && "CreateShaderCompiler failed");

    char const* glslMsl = R"(
        #version 450
        layout(location=0) in vec2 vPos;
        void main() { gl_Position = vec4(vPos, 0.0, 1.0); }
    )";
    te::shader::IShaderHandle* handleMsl = compiler->LoadSourceFromMemory(glslMsl, std::strlen(glslMsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handleMsl && "LoadSourceFromMemory failed");
    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::MSL;
    opts.stage = te::shader::ShaderStage::Vertex;
    bool ok = compiler->Compile(handleMsl, opts);
    if (!ok) {
        std::printf("GLSL->MSL failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handleMsl);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }
    size_t mslSize = 0;
    void const* mslCode = compiler->GetBytecode(handleMsl, &mslSize);
    assert(mslCode && mslSize > 0 && "GetBytecode MSL failed");
    std::printf("te_shader test_msl_hlsl_source: MSL %zu bytes\n", mslSize);
    compiler->ReleaseHandle(handleMsl);

    char const* glslHlsl = R"(
        #version 450
        void main() { gl_Position = vec4(0.0, 0.0, 0.0, 1.0); }
    )";
    te::shader::IShaderHandle* handleHlsl = compiler->LoadSourceFromMemory(glslHlsl, std::strlen(glslHlsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handleHlsl && "LoadSourceFromMemory failed");
    opts.targetBackend = te::shader::BackendType::HLSL_SOURCE;
    opts.stage = te::shader::ShaderStage::Vertex;
    ok = compiler->Compile(handleHlsl, opts);
    if (!ok) {
        std::printf("GLSL->HLSL_SOURCE failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handleHlsl);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }
    size_t hlslSize = 0;
    void const* hlslCode = compiler->GetBytecode(handleHlsl, &hlslSize);
    assert(hlslCode && hlslSize > 0 && "GetBytecode HLSL_SOURCE failed");
    std::printf("te_shader test_msl_hlsl_source: HLSL_SOURCE %zu bytes\n", hlslSize);

    compiler->ReleaseHandle(handleHlsl);
    te::shader::DestroyShaderCompiler(compiler);
    std::printf("te_shader test_msl_hlsl_source: all OK\n");
    return 0;
}
