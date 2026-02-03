// 009-RenderCore PassProtocol (te::rendercore)

#include <te/rendercore/pass_protocol.hpp>

namespace te {
namespace rendercore {

void DeclareRead(PassHandle pass, ResourceHandle resource) {
    (void)pass;
    (void)resource;
}

void DeclareWrite(PassHandle pass, ResourceHandle resource) {
    (void)pass;
    (void)resource;
}

void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime) {
    decl.lifetime = lifetime;
}

} // namespace rendercore
} // namespace te
