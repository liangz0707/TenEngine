#include <te/shader/detail/handle_impl.hpp>
#include <cstring>

namespace te::shader {

namespace {

uint64_t hashMacroSet(MacroSet const& m) {
    uint64_t h = 0;
    for (uint32_t i = 0; i < m.count && i < MacroSet::kMaxPairs; ++i) {
        for (size_t j = 0; m.names[i][j] != '\0'; ++j)
            h = h * 31u + static_cast<unsigned char>(m.names[i][j]);
        h = h * 31u + '=';
        for (size_t j = 0; m.values[i][j] != '\0'; ++j)
            h = h * 31u + static_cast<unsigned char>(m.values[i][j]);
    }
    return h;
}

}  // namespace

void ShaderHandleImpl::SetMacros(MacroSet const& macros) {
    macros_ = macros;
    currentKey_.hash = hashMacroSet(macros);
}

VariantKey ShaderHandleImpl::GetVariantKey() const {
    return currentKey_;
}

void ShaderHandleImpl::SelectVariant(VariantKey key) {
    currentKey_ = key;
    auto it = variantMacros_.find(key.hash);
    if (it != variantMacros_.end()) {
        macros_ = it->second;
    }
}

}  // namespace te::shader
