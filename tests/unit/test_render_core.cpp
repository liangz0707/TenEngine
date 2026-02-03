// 009-RenderCore Unit Tests
// Tests for contract API using only public types.

#include <te/rendercore/api.hpp>
#include <cassert>
#include <cstdio>

using namespace te::rendercore;

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

void TestCreateUniformLayout() {
    UniformMember members[] = {
        {"MVP", UniformMemberType::Mat4, 0, 64},
        {"color", UniformMemberType::Float4, 64, 16}
    };
    UniformLayoutDesc desc{members, 2, 80};
    IUniformLayout* layout = CreateUniformLayout(desc);
    assert(layout != nullptr);

    size_t mvpOffset = layout->GetOffset("MVP");
    assert(mvpOffset == 0);

    size_t colorOffset = layout->GetOffset("color");
    assert(colorOffset == 64);

    size_t unknownOffset = layout->GetOffset("unknown");
    assert(unknownOffset == 0); // Not found

    ReleaseUniformLayout(layout);
    std::printf("[PASS] TestCreateUniformLayout\n");
}

void TestPassProtocol() {
    PassHandle pass{1};
    ResourceHandle res{100};
    DeclareRead(pass, res);
    DeclareWrite(pass, res);

    PassResourceDecl decl{};
    decl.pass = pass;
    decl.resource = res;
    SetResourceLifetime(decl, ResourceLifetime::Persistent);
    assert(decl.lifetime == ResourceLifetime::Persistent);

    std::printf("[PASS] TestPassProtocol\n");
}

void TestUniformBuffer() {
    UniformMember members[] = {
        {"data", UniformMemberType::Float4, 0, 16}
    };
    UniformLayoutDesc layoutDesc{members, 1, 16};
    IUniformLayout* layout = CreateUniformLayout(layoutDesc);
    assert(layout != nullptr);

    IUniformBuffer* ub = CreateUniformBuffer(layout, nullptr);
    assert(ub != nullptr);

    // Update on slot 0
    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    ub->SetCurrentFrameSlot(0);
    ub->Update(data, sizeof(data));

    // Switch slot and update again
    ub->SetCurrentFrameSlot(1);
    ub->Update(data, sizeof(data));

    // Bind (no-op placeholder)
    ub->Bind(nullptr, 0);

    ReleaseUniformBuffer(ub);
    ReleaseUniformLayout(layout);

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
    TestCreateUniformLayout();
    TestPassProtocol();
    TestUniformBuffer();

    std::printf("=== All tests passed ===\n");
    return 0;
}
