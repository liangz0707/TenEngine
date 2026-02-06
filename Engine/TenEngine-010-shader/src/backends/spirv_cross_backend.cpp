#include <te/shader/detail/spirv_cross_backend.hpp>
#include <te/shader/detail/handle_impl.hpp>

#if TENENGINE_USE_SPIRV_CROSS
#include <spirv_cross.hpp>
#include <spirv_hlsl.hpp>
#include <spirv_msl.hpp>
#endif

#include <string>

namespace te::shader {

#if TENENGINE_USE_SPIRV_CROSS

namespace {

bool compileSpirvToMsl(uint32_t const* spirv, size_t wordCount, std::string& outSource, std::string& outError) {
    try {
        spirv_cross::CompilerMSL msl(spirv, wordCount);
        spirv_cross::CompilerMSL::Options opts;
        opts.platform = spirv_cross::CompilerMSL::Options::macOS;
        msl.set_msl_options(opts);
        outSource = msl.compile();
        return true;
    } catch (std::exception const& e) {
        outError = "SPIRV-Cross MSL: ";
        outError += e.what();
        return false;
    }
}

bool compileSpirvToHlsl(uint32_t const* spirv, size_t wordCount, std::string& outSource, std::string& outError) {
    try {
        spirv_cross::CompilerHLSL hlsl(spirv, wordCount);
        spirv_cross::CompilerHLSL::Options opts;
        opts.shader_model = 50;
        hlsl.set_hlsl_options(opts);
        outSource = hlsl.compile();
        return true;
    } catch (std::exception const& e) {
        outError = "SPIRV-Cross HLSL: ";
        outError += e.what();
        return false;
    }
}

}  // namespace

bool CompileSpirvToTarget(ShaderHandleImpl* handle, SpirvCrossTarget target, std::string& outError) {
    if (!handle || handle->bytecode_.empty()) {
        outError = "No SPIR-V bytecode to cross-compile";
        return false;
    }
    std::string src;
    bool ok = (target == SpirvCrossTarget::MSL)
                  ? compileSpirvToMsl(handle->bytecode_.data(), handle->bytecode_.size(), src, outError)
                  : compileSpirvToHlsl(handle->bytecode_.data(), handle->bytecode_.size(), src, outError);
    if (ok) {
        handle->crossCompiledSource_ = std::move(src);
    }
    return ok;
}

#else

bool CompileSpirvToTarget(ShaderHandleImpl* handle, SpirvCrossTarget /*target*/, std::string& outError) {
    (void)handle;
    outError = "SPIRV-Cross not enabled (TENENGINE_USE_SPIRV_CROSS=OFF)";
    return false;
}

#endif

}  // namespace te::shader
