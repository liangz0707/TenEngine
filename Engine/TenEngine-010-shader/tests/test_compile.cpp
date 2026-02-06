#include <te/shader/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    assert(compiler && "CreateShaderCompiler failed");

    const char* glsl = R"(
        #version 450
        void main() { }
    )";
    te::shader::IShaderHandle* handle = compiler->LoadSourceFromMemory(glsl, std::strlen(glsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handle && "LoadSourceFromMemory failed");

    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::SPIRV;
    bool ok = compiler->Compile(handle, opts);
    if (!ok) {
        std::printf("Compile failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handle);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }

    size_t size = 0;
    void const* bytecode = compiler->GetBytecode(handle, &size);
    assert(bytecode && size > 0 && "GetBytecode failed");

    std::printf("te_shader test_compile: OK (bytecode %zu bytes)\n", size);

    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCompiler(compiler);
    std::printf("te_shader test_compile: all OK\n");
    return 0;
}
