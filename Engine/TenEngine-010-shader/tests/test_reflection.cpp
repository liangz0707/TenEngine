#include <te/shader/api.hpp>
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/shader_reflection.hpp>
#endif
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    assert(compiler && "CreateShaderCompiler failed");

    char const* glsl = R"(
        #version 450
        layout(binding=0) uniform Uniforms { mat4 uMVP; vec4 uColor; };
        layout(binding=1) uniform texture2D uTex;
        layout(binding=2) uniform sampler uSamp;
        layout(location=0) in vec2 vUV;
        layout(location=0) out vec4 outColor;
        void main() {
            outColor = uColor * texture(sampler2D(uTex, uSamp), vUV);
        }
    )";
    te::shader::IShaderHandle* handle = compiler->LoadSourceFromMemory(glsl, std::strlen(glsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handle && "LoadSourceFromMemory failed");

    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::SPIRV;
    opts.stage = te::shader::ShaderStage::Fragment;
    bool ok = compiler->Compile(handle, opts);
    if (!ok) {
        std::printf("Compile failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handle);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }

#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    te::rendercore::UniformLayoutDesc uniformDesc{};
    bool hasRefl = compiler->GetReflection(handle, &uniformDesc);
    if (hasRefl) {
        std::printf("te_shader test_reflection: UniformLayoutDesc memberCount=%u totalSize=%u\n",
                    uniformDesc.memberCount, uniformDesc.totalSize);
        assert(uniformDesc.memberCount > 0 && uniformDesc.totalSize > 0);
    }

    te::rendercore::ShaderReflectionDesc shaderDesc{};
    bool hasShaderRefl = compiler->GetShaderReflection(handle, &shaderDesc);
    assert(hasShaderRefl && "GetShaderReflection expected true");
    std::printf("te_shader test_reflection: ShaderReflectionDesc uniformBlock.memberCount=%u resourceBindingCount=%u\n",
                shaderDesc.uniformBlock.memberCount, shaderDesc.resourceBindingCount);
    assert(shaderDesc.uniformBlock.memberCount > 0 || shaderDesc.resourceBindingCount > 0);
#else
    (void)opts;
    std::printf("te_shader test_reflection: TE_SHADER_USE_CORE not set, skip reflection checks\n");
#endif

    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCompiler(compiler);
    std::printf("te_shader test_reflection: all OK\n");
    return 0;
}
