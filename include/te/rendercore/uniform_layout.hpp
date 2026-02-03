#pragma once
// 009-RenderCore UniformLayout API (te::rendercore)
// ABI: specs/_contracts/009-rendercore-ABI.md

#include <te/rendercore/types.hpp>
#include <cstddef>

namespace te {
namespace rendercore {

class IUniformLayout {
public:
    virtual ~IUniformLayout() = default;
    virtual size_t GetOffset(char const* name) const = 0;
    virtual size_t GetTotalSize() const = 0;
};

IUniformLayout* CreateUniformLayout(UniformLayoutDesc const& desc);
void ReleaseUniformLayout(IUniformLayout* layout);

} // namespace rendercore
} // namespace te
