#ifndef TE_SHADER_TYPES_HPP
#define TE_SHADER_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace te::shader {

enum class ShaderSourceFormat : uint32_t {
    HLSL,
    GLSL,
};

enum class BackendType : uint32_t {
    SPIRV,
    DXIL,
    MSL,
    HLSL_SOURCE,  // D3D11: SPIR-V -> HLSL source via SPIRV-Cross
};

struct MacroSet {
    static constexpr size_t kMaxPairs = 32;
    char names[kMaxPairs][64];
    char values[kMaxPairs][64];
    uint32_t count = 0;
};

struct VariantKey {
    uint64_t hash = 0;
    bool operator==(VariantKey const& o) const { return hash == o.hash; }
    bool operator!=(VariantKey const& o) const { return hash != o.hash; }
};

struct CompileOptions {
    BackendType targetBackend = BackendType::SPIRV;
    uint32_t optimizationLevel = 1;
    bool generateDebugInfo = false;
};

class IVariantEnumerator {
public:
    virtual ~IVariantEnumerator() = default;
    virtual void OnVariant(VariantKey key) = 0;
};

using SourceChangedCallback = void (*)(char const* path, void* userData);

}  // namespace te::shader

#endif  // TE_SHADER_TYPES_HPP
