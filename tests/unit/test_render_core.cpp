// 009-RenderCore Unit Tests (ABI: te::rendercore)

#include <te/rendercore/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace te::rendercore;

// ============================================================================
// ResourceDesc
// ============================================================================

static void TestCreateVertexFormat() {
    VertexAttribute attrs[] = {
        {0, VertexAttributeFormat::Float3, 0},
        {1, VertexAttributeFormat::Float2, 12}
    };
    VertexFormatDesc desc{attrs, 2, 20};
    VertexFormat vf = CreateVertexFormat(desc);
    assert(vf.IsValid());
    assert(vf.attributeCount == 2);
    assert(vf.stride == 20);

    VertexFormatDesc invalidDesc{nullptr, 0, 20};
    VertexFormat invalidVf = CreateVertexFormat(invalidDesc);
    assert(!invalidVf.IsValid());

    std::printf("[PASS] TestCreateVertexFormat\n");
}

static void TestCreateIndexFormat() {
    IndexFormatDesc desc16{IndexType::UInt16};
    IndexFormat idx16 = CreateIndexFormat(desc16);
    assert(idx16.IsValid());
    assert(idx16.type == IndexType::UInt16);

    IndexFormatDesc desc32{IndexType::UInt32};
    IndexFormat idx32 = CreateIndexFormat(desc32);
    assert(idx32.IsValid());

    IndexFormatDesc descUnknown{IndexType::Unknown};
    IndexFormat idxUnknown = CreateIndexFormat(descUnknown);
    assert(!idxUnknown.IsValid());

    std::printf("[PASS] TestCreateIndexFormat\n");
}

static void TestCreateTextureDesc() {
    TextureDescParams params{1024, 768, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
    TextureDesc tex = CreateTextureDesc(params);
    assert(tex.IsValid());
    assert(tex.width == 1024 && tex.height == 768);

    TextureDescParams invalidParams{0, 768, 1, 1, TextureFormat::RGBA8_UNorm, TextureUsage::Sampled};
    TextureDesc invalidTex = CreateTextureDesc(invalidParams);
    assert(!invalidTex.IsValid());

    std::printf("[PASS] TestCreateTextureDesc\n");
}

static void TestCreateBufferDesc() {
    BufferDescParams params{4096, BufferUsage::Vertex, 16};
    BufferDesc buf = CreateBufferDesc(params);
    assert(buf.IsValid());
    assert(buf.size == 4096);

    BufferDescParams invalidParams{0, BufferUsage::Vertex, 16};
    BufferDesc invalidBuf = CreateBufferDesc(invalidParams);
    assert(!invalidBuf.IsValid());

    std::printf("[PASS] TestCreateBufferDesc\n");
}

// ============================================================================
// IUniformLayout, CreateUniformLayout, GetOffset, GetTotalSize
// ============================================================================

static void TestUniformLayout() {
    UniformMember members[2] = {};
    std::strncpy(members[0].name, "MVP", 63); members[0].name[63] = '\0';
    members[0].type = UniformMemberType::Mat4; members[0].offset = 0; members[0].size = 64;
    std::strncpy(members[1].name, "color", 63); members[1].name[63] = '\0';
    members[1].type = UniformMemberType::Float4; members[1].offset = 64; members[1].size = 16;
    UniformLayoutDesc desc{members, 2, 80};
    IUniformLayout* layout = CreateUniformLayout(desc);
    assert(layout != nullptr);

    assert(layout->GetOffset("MVP") == 0u);
    assert(layout->GetOffset("color") == 64u);
    assert(layout->GetOffset("unknown") == 0u);
    assert(layout->GetTotalSize() >= 80u);

    ReleaseUniformLayout(layout);
    std::printf("[PASS] TestUniformLayout\n");
}

// ============================================================================
// PassProtocol (DeclareRead, DeclareWrite, SetResourceLifetime)
// ============================================================================

static void TestPassProtocol() {
    PassHandle pass{1};
    ResourceHandle res{100};

    DeclareRead(pass, res);
    DeclareWrite(pass, res);

    PassResourceDecl decl{pass, res, true, false, ResourceLifetime::Transient};
    SetResourceLifetime(decl, ResourceLifetime::Persistent);
    assert(decl.lifetime == ResourceLifetime::Persistent);

    std::printf("[PASS] TestPassProtocol\n");
}

// ============================================================================
// UniformBuffer (CreateUniformBuffer, Update, GetRingBufferOffset, Bind)
// ============================================================================

static void TestUniformBuffer() {
    UniformMember members[1] = {};
    std::strncpy(members[0].name, "data", 63); members[0].name[63] = '\0';
    members[0].type = UniformMemberType::Float4; members[0].offset = 0; members[0].size = 16;
    UniformLayoutDesc layoutDesc{members, 1, 16};
    IUniformLayout* layout = CreateUniformLayout(layoutDesc);
    assert(layout != nullptr);

    // Without a real RHI device, CreateUniformBuffer returns nullptr (per ABI: direct RHI calls).
    IUniformBuffer* ub = CreateUniformBuffer(layout, nullptr);
    if (!ub) {
        ReleaseUniformLayout(layout);
        std::printf("[SKIP] TestUniformBuffer (no device; RHI required)\n");
        return;
    }

    float data[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    ub->Update(data, sizeof(data));

    size_t off0 = ub->GetRingBufferOffset(0);
    size_t off1 = ub->GetRingBufferOffset(1);
    assert(off1 > off0);

    ub->Bind(nullptr, 0);

    ReleaseUniformBuffer(ub);
    ReleaseUniformLayout(layout);

    std::printf("[PASS] TestUniformBuffer\n");
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::printf("=== 009-RenderCore Unit Tests (te::rendercore) ===\n");

    TestCreateVertexFormat();
    TestCreateIndexFormat();
    TestCreateTextureDesc();
    TestCreateBufferDesc();
    TestUniformLayout();
    TestPassProtocol();
    TestUniformBuffer();

    std::printf("=== All tests passed ===\n");
    return 0;
}
