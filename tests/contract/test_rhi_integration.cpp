// 009-RenderCore Contract Tests
// Verifies descriptors are accepted by RHI creation (per 008-RHI contract).
// Note: Requires RHI test harness; placeholder for now.

#include "render_core/api.hpp"
#include <cassert>
#include <cstdio>

using namespace TenEngine::RenderCore;

// ============================================================================
// T021: Contract tests (RHI integration)
// ============================================================================

// Note: These tests require the 008-RHI module to be available.
// When integrated with the main TenEngine repo, uncomment and run.

void TestVertexFormatAcceptedByRHI() {
    // Create a VertexFormat
    VertexAttribute attrs[] = {
        {0, VertexAttributeFormat::Float3, 0},  // position
        {1, VertexAttributeFormat::Float2, 12}  // texcoord
    };
    VertexFormatDesc desc{attrs, 2, 20};
    VertexFormat vf = CreateVertexFormat(desc);
    assert(vf.IsValid());

    // TODO: When RHI is available:
    // RHI::VertexBufferHandle vbh = RHI::CreateVertexBuffer(vf, ...);
    // assert(vbh.IsValid());

    std::printf("[PASS] TestVertexFormatAcceptedByRHI (placeholder)\n");
}

void TestTextureDescAcceptedByRHI() {
    TextureDescParams params{1024, 1024, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
    TextureDesc td = CreateTextureDesc(params);
    assert(td.IsValid());

    // TODO: When RHI is available:
    // RHI::TextureHandle th = RHI::CreateTexture(td, ...);
    // assert(th.IsValid());

    std::printf("[PASS] TestTextureDescAcceptedByRHI (placeholder)\n");
}

void TestBufferDescAcceptedByRHI() {
    BufferDescParams params{4096, BufferUsage::Uniform, 256};
    BufferDesc bd = CreateBufferDesc(params);
    assert(bd.IsValid());

    // TODO: When RHI is available:
    // RHI::BufferHandle bh = RHI::CreateBuffer(bd, ...);
    // assert(bh.IsValid());

    std::printf("[PASS] TestBufferDescAcceptedByRHI (placeholder)\n");
}

int main() {
    std::printf("=== 009-RenderCore Contract Tests ===\n");

    TestVertexFormatAcceptedByRHI();
    TestTextureDescAcceptedByRHI();
    TestBufferDescAcceptedByRHI();

    std::printf("=== Contract tests completed (RHI integration pending) ===\n");
    return 0;
}
