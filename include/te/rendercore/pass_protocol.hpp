#pragma once
// 009-RenderCore PassProtocol API (te::rendercore)
// ABI: specs/_contracts/009-rendercore-ABI.md

#include <te/rendercore/types.hpp>

namespace te {
namespace rendercore {

void DeclareRead(PassHandle pass, ResourceHandle resource);
void DeclareWrite(PassHandle pass, ResourceHandle resource);
void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime);

} // namespace rendercore
} // namespace te
