// 009-RenderCore Unit Tests
// Tests for contract API using only public types.

#include "render_core/api.hpp"
#include <cassert>
#include <cstdio>

using namespace TenEngine::RenderCore;

// ============================================================================
// T020: Unit tests for descriptors and layout
// ============================================================================

void TestCreateVertexFormat() {
    // Valid format
    VertexAttribute attrs[] = {
        {0, VertexAttributeFormat::Float3, 0},
        {1, VertexAttributeFormat::Float2, 12}
    };
    VertexFormatDesc desc{attrs, 2, 20};
    VertexFormat vf = CreateVertexFormat(desc);
    assert(vf.IsValid());
    assert(vf.attributeCount == 2);
    assert(vf.stride == 20);

    // Invalid: null attributes
    VertexFormatDesc invalidDesc{nullptr, 0, 20};
    VertexFormat invalidVf = CreateVertexFormat(invalidDesc);
    assert(!invalidVf.IsValid());

    std::printf("[PASS] TestCreateVertexFormat\n");
}

void TestCreateIndexFormat() {
    // Valid: UInt16
    IndexFormatDesc desc16{IndexType::UInt16};
    IndexFormat idx16 = CreateIndexFormat(desc16);
    assert(idx16.IsValid());
    assert(idx16.type == IndexType::UInt16);

    // Valid: UInt32
    IndexFormatDesc desc32{IndexType::UInt32};
    IndexFormat idx32 = CreateIndexFormat(desc32);
    assert(idx32.IsValid());
    assert(idx32.type == IndexType::UInt32);

    // Invalid: Unknown
    IndexFormatDesc descUnknown{IndexType::Unknown};
    IndexFormat idxUnknown = CreateIndexFormat(descUnknown);
    assert(!idxUnknown.IsValid());

    std::printf("[PASS] TestCreateIndexFormat\n");
}

void TestCreateTextureDesc() {
    // Valid
    TextureDescParams params{1024, 768, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
    TextureDesc tex = CreateTextureDesc(params);
    assert(tex.IsValid());
    assert(tex.width == 1024);
    assert(tex.height == 768);

    // Invalid: zero width
    TextureDescParams invalidParams{0, 768, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
    TextureDesc invalidTex = CreateTextureDesc(invalidParams);
    assert(!invalidTex.IsValid());

    std::printf("[PASS] TestCreateTextureDesc\n");
}

void TestCreateBufferDesc() {
    // Valid
    BufferDescParams params{4096, BufferUsage::Vertex, 16};
    BufferDesc buf = CreateBufferDesc(params);
    assert(buf.IsValid());
    assert(buf.size == 4096);

    // Invalid: zero size
    BufferDescParams invalidParams{0, BufferUsage::Vertex, 16};
    BufferDesc invalidBuf = CreateBufferDesc(invalidParams);
    assert(!invalidBuf.IsValid());

    std::printf("[PASS] TestCreateBufferDesc\n");
}

void TestDefineLayoutAndGetOffset() {
    UniformMember members[] = {
        {"MVP", UniformMemberType::Mat4, 0, 64},
        {"color", UniformMemberType::Float4, 64, 16}
    };
    UniformLayoutDesc desc{members, 2, 80};
    UniformLayout layout = DefineLayout(desc);
    assert(layout.IsValid());

    size_t mvpOffset = GetOffset(layout, "MVP");
    assert(mvpOffset == 0);

    size_t colorOffset = GetOffset(layout, "color");
    assert(colorOffset == 64);

    size_t unknownOffset = GetOffset(layout, "unknown");
    assert(unknownOffset == 0); // Not found

    ReleaseLayout(layout);
    std::printf("[PASS] TestDefineLayoutAndGetOffset\n");
}

void TestPassProtocol() {
    PassHandle pass{1};
    ResourceHandle res{100};

    PassResourceDecl readDecl = DeclareRead(pass, res);
    assert(readDecl.IsValid());
    assert(readDecl.isRead);
    assert(!readDecl.isWrite);

    PassResourceDecl writeDecl = DeclareWrite(pass, res);
    assert(writeDecl.IsValid());
    assert(!writeDecl.isRead);
    assert(writeDecl.isWrite);

    SetResourceLifetime(readDecl, ResourceLifetime::Persistent);
    assert(readDecl.lifetime == ResourceLifetime::Persistent);

    // Invalid handles
    PassHandle invalidPass{0};
    PassResourceDecl invalidDecl = DeclareRead(invalidPass, res);
    assert(!invalidDecl.IsValid());

    std::printf("[PASS] TestPassProtocol\n");
}

void TestUniformBuffer() {
    UniformMember members[] = {
        {"data", UniformMemberType::Float4, 0, 16}
    };
    UniformLayoutDesc layoutDesc{members, 1, 16};
    UniformLayout layout = DefineLayout(layoutDesc);
    assert(layout.IsValid());

    UniformBufferHandle ub = CreateUniformBuffer(layout);
    assert(ub.IsValid());

    // Update
    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    Update(ub, data, sizeof(data));

    // RingBuffer
    bool advanced = RingBufferAdvance(ub);
    assert(advanced);

    // Bind
    BindSlot slot{0, 0};
    Bind(ub, slot);

    ReleaseUniformBuffer(ub);
    ReleaseLayout(layout);

    std::printf("[PASS] TestUniformBuffer\n");
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::printf("=== 009-RenderCore Unit Tests ===\n");

    TestCreateVertexFormat();
    TestCreateIndexFormat();
    TestCreateTextureDesc();
    TestCreateBufferDesc();
    TestDefineLayoutAndGetOffset();
    TestPassProtocol();
    TestUniformBuffer();

    std::printf("=== All tests passed ===\n");
    return 0;
}
