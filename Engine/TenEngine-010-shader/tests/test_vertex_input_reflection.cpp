#include <te/shader/api.hpp>
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
#include <te/rendercore/resource_desc.hpp>
#endif
#include <cassert>
#include <cstdio>
#include <cstring>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    assert(compiler && "CreateShaderCompiler failed");

    char const* glsl = R"(
        #version 450
        layout(location=0) in vec3 aPos;
        layout(location=1) in vec2 aUV;
        void main() { gl_Position = vec4(aPos, 1.0); }
    )";
    te::shader::IShaderHandle* handle = compiler->LoadSourceFromMemory(glsl, std::strlen(glsl), te::shader::ShaderSourceFormat::GLSL);
    assert(handle && "LoadSourceFromMemory failed");

    te::shader::CompileOptions opts{};
    opts.targetBackend = te::shader::BackendType::SPIRV;
    opts.stage = te::shader::ShaderStage::Vertex;
    bool ok = compiler->Compile(handle, opts);
    if (!ok) {
        std::printf("Compile failed: %s\n", compiler->GetLastError());
        compiler->ReleaseHandle(handle);
        te::shader::DestroyShaderCompiler(compiler);
        return 1;
    }

#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    te::rendercore::VertexFormatDesc vertexDesc{};
    bool hasVertexRefl = compiler->GetVertexInputReflection(handle, &vertexDesc);
    assert(hasVertexRefl && "GetVertexInputReflection expected true for vertex shader");
    assert(vertexDesc.attributes && vertexDesc.attributeCount >= 2 && "expected at least 2 vertex attributes");
    assert(vertexDesc.stride > 0 && "expected positive stride");

    bool hasPos = false, hasUV = false;
    for (uint32_t i = 0; i < vertexDesc.attributeCount; ++i) {
        if (vertexDesc.attributes[i].location == 0 && vertexDesc.attributes[i].format == te::rendercore::VertexAttributeFormat::Float3)
            hasPos = true;
        if (vertexDesc.attributes[i].location == 1 && vertexDesc.attributes[i].format == te::rendercore::VertexAttributeFormat::Float2)
            hasUV = true;
    }
    assert(hasPos && hasUV && "expected location 0 vec3 and location 1 vec2");
    std::printf("te_shader test_vertex_input_reflection: attributeCount=%u stride=%u\n",
                vertexDesc.attributeCount, vertexDesc.stride);
#else
    (void)opts;
    std::printf("te_shader test_vertex_input_reflection: TE_SHADER_USE_CORE not set, skip vertex reflection\n");
#endif

    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCompiler(compiler);
    std::printf("te_shader test_vertex_input_reflection: all OK\n");
    return 0;
}
