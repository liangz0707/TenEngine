// 009-RenderCore PassProtocol Implementation
// Contract: specs/_contracts/009-rendercore-public-api.md §3. PassProtocol

#include "pass_protocol.hpp"

namespace TenEngine::RenderCore {

// ============================================================================
// T011: DeclareRead
// ============================================================================

PassResourceDecl DeclareRead(PassHandle pass, ResourceHandle resource) {
    PassResourceDecl decl{};

    // Validate handles
    if (!pass.IsValid() || !resource.IsValid()) {
        return decl; // Invalid declaration
    }

    decl.pass = pass;
    decl.resource = resource;
    decl.isRead = true;
    decl.isWrite = false;
    decl.lifetime = ResourceLifetime::Transient;

    return decl;
}

// ============================================================================
// T012: DeclareWrite
// ============================================================================

PassResourceDecl DeclareWrite(PassHandle pass, ResourceHandle resource) {
    PassResourceDecl decl{};

    // Validate handles
    if (!pass.IsValid() || !resource.IsValid()) {
        return decl; // Invalid declaration
    }

    decl.pass = pass;
    decl.resource = resource;
    decl.isRead = false;
    decl.isWrite = true;
    decl.lifetime = ResourceLifetime::Transient;

    return decl;
}

// ============================================================================
// T013: SetResourceLifetime
// ============================================================================

void SetResourceLifetime(PassResourceDecl& decl, ResourceLifetime lifetime) {
    // Note: Same-resource read+write in one pass is defined by PipelineCore only.
    // RenderCore does not add restriction per spec clarification.
    decl.lifetime = lifetime;
}

} // namespace TenEngine::RenderCore
