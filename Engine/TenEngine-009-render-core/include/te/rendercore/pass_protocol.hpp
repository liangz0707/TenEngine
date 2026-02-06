/** @file pass_protocol.hpp
 *  009-RenderCore ABI: PassResourceDecl, DeclareRead, DeclareWrite,
 *  SetResourceLifetime.
 */
#pragma once

#include <te/rendercore/types.hpp>

namespace te {
namespace rendercore {

struct PassResourceDecl {
  PassHandle pass;
  ResourceHandle resource;
  bool isRead = false;
  bool isWrite = false;
  ResourceLifetime lifetime = ResourceLifetime::Transient;
  bool IsValid() const { return pass.IsValid() && resource.IsValid(); }
};

void DeclareRead(PassHandle pass, ResourceHandle resource);
void DeclareWrite(PassHandle pass, ResourceHandle resource);
void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime);

}  // namespace rendercore
}  // namespace te
