#ifndef TE_SHADER_DETAIL_HANDLE_IMPL_HPP
#define TE_SHADER_DETAIL_HANDLE_IMPL_HPP

#include <te/shader/handle.hpp>
#include <te/shader/types.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/shader_reflection.hpp>
#endif

namespace te::shader {

struct VariantBytecode {
    std::vector<uint32_t> spirv;
    std::vector<uint8_t> dxil;
    std::string crossCompiled;
};

class ShaderHandleImpl : public IShaderHandle {
public:
    MacroSet macros_{};
    VariantKey currentKey_{};
    std::vector<uint32_t> bytecode_;       // SPIR-V (legacy / current)
    std::vector<uint8_t> bytecodeBlob_;    // DXIL (legacy / current)
    std::string crossCompiledSource_;      // MSL/HLSL (legacy / current)
    std::string sourcePath_;
    std::string sourceCode_;
    ShaderSourceFormat sourceFormat_ = ShaderSourceFormat::GLSL;
    std::unordered_map<uint64_t, MacroSet> variantMacros_;   // key.hash -> MacroSet for SelectVariant
    std::unordered_map<uint64_t, VariantBytecode> variantBytecode_;  // per-variant cache
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    std::vector<te::rendercore::UniformMember> reflectionMembers_;
    uint32_t reflectionTotalSize_ = 0;
    std::vector<te::rendercore::ShaderResourceBinding> reflectionResourceBindings_;
#endif

    void SetMacros(MacroSet const& macros) override;
    VariantKey GetVariantKey() const override;
    void SelectVariant(VariantKey key) override;
};

}  // namespace te::shader

#endif
