#pragma once
// 009-RenderCore PassProtocol API
// Contract: specs/_contracts/009-rendercore-public-api.md §3. PassProtocol

#include "types.hpp"

namespace TenEngine::RenderCore {

/// Declare read resource for a pass.
/// Returns PassResourceDecl for the pass graph.
PassResourceDecl DeclareRead(PassHandle pass, ResourceHandle resource);

/// Declare write resource for a pass.
/// Returns PassResourceDecl for the pass graph.
PassResourceDecl DeclareWrite(PassHandle pass, ResourceHandle resource);

/// Set resource lifetime for a declaration.
/// Aligns with PipelineCore RDG protocol and RHI constraints.
/// Same-resource read+write in one pass is defined by PipelineCore only.
void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime);

} // namespace TenEngine::RenderCore
