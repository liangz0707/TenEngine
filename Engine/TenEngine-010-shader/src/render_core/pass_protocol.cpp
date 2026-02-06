// 009-RenderCore Pass Protocol (ABI: te/rendercore/pass_protocol.hpp)

#include <te/rendercore/pass_protocol.hpp>

namespace te {
namespace rendercore {

namespace {
thread_local PassResourceDecl g_lastDeclaration{};
}

void DeclareRead(PassHandle pass, ResourceHandle resource) {
  if (!pass.IsValid() || !resource.IsValid()) {
    g_lastDeclaration = {};
    return;
  }

  g_lastDeclaration.pass = pass;
  g_lastDeclaration.resource = resource;
  g_lastDeclaration.isRead = true;
  g_lastDeclaration.isWrite = false;
  g_lastDeclaration.lifetime = ResourceLifetime::Transient;
}

void DeclareWrite(PassHandle pass, ResourceHandle resource) {
  if (!pass.IsValid() || !resource.IsValid()) {
    g_lastDeclaration = {};
    return;
  }

  g_lastDeclaration.pass = pass;
  g_lastDeclaration.resource = resource;
  g_lastDeclaration.isRead = false;
  g_lastDeclaration.isWrite = true;
  g_lastDeclaration.lifetime = ResourceLifetime::Transient;
}

void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime) {
  decl.lifetime = lifetime;
}

}  // namespace rendercore
}  // namespace te
