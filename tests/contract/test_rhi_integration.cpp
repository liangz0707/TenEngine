// 009-RenderCore Contract Tests (te::rendercore)
// Verifies descriptors are accepted by RHI creation (per 008-RHI contract).

#include <te/rendercore/api.hpp>
#include <cassert>
#include <cstdio>

using namespace te::rendercore;

void TestVertexFormatAcceptedByRHI() {
    VertexAttribute attrs[] = {
        {0, VertexAttributeFormat::Float3, 0},
        {1, VertexAttributeFormat::Float2, 12}
    };
    VertexFormatDesc desc{attrs, 2, 20};
    VertexFormat vf = CreateVertexFormat(desc);
    assert(vf.IsValid());

    std::printf("[PASS] TestVertexFormatAcceptedByRHI (placeholder)\n");
}

void TestTextureDescAcceptedByRHI() {
    TextureDescParams params{1024, 1024, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
    TextureDesc td = CreateTextureDesc(params);
    assert(td.IsValid());

    std::printf("[PASS] TestTextureDescAcceptedByRHI (placeholder)\n");
}

void TestBufferDescAcceptedByRHI() {
    BufferDescParams params{4096, BufferUsage::Uniform, 256};
    BufferDesc bd = CreateBufferDesc(params);
    assert(bd.IsValid());

    std::printf("[PASS] TestBufferDescAcceptedByRHI (placeholder)\n");
}

int main() {
    std::printf("=== 009-RenderCore Contract Tests (te::rendercore) ===\n");

    TestVertexFormatAcceptedByRHI();
    TestTextureDescAcceptedByRHI();
    TestBufferDescAcceptedByRHI();

    std::printf("=== Contract tests completed (RHI integration pending) ===\n");
    return 0;
}
