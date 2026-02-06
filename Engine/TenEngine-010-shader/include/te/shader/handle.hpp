#ifndef TE_SHADER_HANDLE_HPP
#define TE_SHADER_HANDLE_HPP

#include <te/shader/types.hpp>

namespace te::shader {

class IShaderHandle {
public:
    virtual ~IShaderHandle() = default;
    virtual void SetMacros(MacroSet const& macros) = 0;
    virtual VariantKey GetVariantKey() const = 0;
    virtual void SelectVariant(VariantKey key) = 0;
};

}  // namespace te::shader

#endif  // TE_SHADER_HANDLE_HPP
