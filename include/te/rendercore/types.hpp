#pragma once
// 009-RenderCore Contract Types (te::rendercore)
// ABI: specs/_contracts/009-rendercore-ABI.md
// No RHI internals; uses only 001-Core and 008-RHI contract types.

#include <cstddef>
#include <cstdint>

namespace te {
namespace rendercore {

// ============================================================================
// Error / Result
// ============================================================================

enum class ResultCode : uint32_t {
    Success = 0,
    InvalidHandle,
    UnsupportedFormat,
    UnsupportedSize,
    ValidationFailed,
    RingBufferExhausted,
    Unknown
};

// ============================================================================
// Forward declarations (internal)
// ============================================================================

struct UniformLayoutImpl;
struct UniformBufferImpl;

// ============================================================================
// Opaque Handles (internal use)
// ============================================================================

struct UniformLayout {
    UniformLayoutImpl* impl = nullptr;
    bool IsValid() const { return impl != nullptr; }
};

struct UniformBufferHandle {
    UniformBufferImpl* impl = nullptr;
    bool IsValid() const { return impl != nullptr; }
};

// ============================================================================
// Resource Lifetime (PassProtocol)
// ============================================================================

enum class ResourceLifetime : uint8_t {
    Transient,
    Persistent,
    External
};

// ============================================================================
// PassHandle / ResourceHandle
// ============================================================================

struct PassHandle {
    uint64_t id = 0;
    bool IsValid() const { return id != 0; }
};

struct ResourceHandle {
    uint64_t id = 0;
    bool IsValid() const { return id != 0; }
};

struct BindSlot {
    uint32_t set = 0;
    uint32_t binding = 0;
};

using FrameSlotId = uint32_t;

// ============================================================================
// PassResourceDecl
// ============================================================================

struct PassResourceDecl {
    PassHandle pass;
    ResourceHandle resource;
    bool isRead = false;
    bool isWrite = false;
    ResourceLifetime lifetime = ResourceLifetime::Transient;

    bool IsValid() const { return pass.IsValid() && resource.IsValid(); }
};

// ============================================================================
// VertexFormat / IndexFormat (ResourceDesc)
// ============================================================================

enum class VertexAttributeFormat : uint8_t {
    Float, Float2, Float3, Float4,
    Int, Int2, Int3, Int4,
    UInt, UInt2, UInt3, UInt4,
    Unknown
};

struct VertexAttribute {
    uint32_t location = 0;
    VertexAttributeFormat format = VertexAttributeFormat::Unknown;
    uint32_t offset = 0;
};

constexpr size_t kMaxVertexAttributes = 16;

struct VertexFormat {
    VertexAttribute attributes[kMaxVertexAttributes];
    uint32_t attributeCount = 0;
    uint32_t stride = 0;

    bool IsValid() const { return attributeCount > 0 && stride > 0; }
};

struct VertexFormatDesc {
    VertexAttribute const* attributes = nullptr;
    uint32_t attributeCount = 0;
    uint32_t stride = 0;
};

enum class IndexType : uint8_t {
    UInt16,
    UInt32,
    Unknown
};

struct IndexFormat {
    IndexType type = IndexType::Unknown;
    bool IsValid() const { return type != IndexType::Unknown; }
};

struct IndexFormatDesc {
    IndexType type = IndexType::Unknown;
};

// ============================================================================
// TextureDesc / BufferDesc (ResourceDesc)
// ============================================================================

enum class TextureFormat : uint16_t {
    Unknown = 0,
    R8_UNorm, RG8_UNorm, RGBA8_UNorm, RGBA8_SRGB, BGRA8_UNorm,
    R16_Float, RG16_Float, RGBA16_Float,
    R32_Float, RG32_Float, RGBA32_Float,
    Depth16, Depth24_Stencil8, Depth32_Float
};

enum class TextureUsage : uint32_t {
    None = 0,
    Sampled = 1 << 0,
    Storage = 1 << 1,
    RenderTarget = 1 << 2,
    DepthStencil = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5
};

inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline TextureUsage operator&(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

struct TextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::Unknown;
    TextureUsage usage = TextureUsage::None;

    bool IsValid() const { return width > 0 && height > 0 && format != TextureFormat::Unknown; }
};

struct TextureDescParams {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::Unknown;
    TextureUsage usage = TextureUsage::None;
};

enum class BufferUsage : uint32_t {
    None = 0,
    Vertex = 1 << 0,
    Index = 1 << 1,
    Uniform = 1 << 2,
    Storage = 1 << 3,
    TransferSrc = 1 << 4,
    TransferDst = 1 << 5
};

inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}
inline BufferUsage operator&(BufferUsage a, BufferUsage b) {
    return static_cast<BufferUsage>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

struct BufferDesc {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    uint32_t alignment = 0;

    bool IsValid() const { return size > 0; }
};

struct BufferDescParams {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    uint32_t alignment = 0;
};

// ============================================================================
// UniformLayoutDesc / Member (UniformLayout)
// ============================================================================

enum class UniformMemberType : uint8_t {
    Float, Float2, Float3, Float4,
    Mat3, Mat4,
    Int, Int2, Int3, Int4,
    Unknown
};

struct UniformMember {
    char name[64] = {};
    UniformMemberType type = UniformMemberType::Unknown;
    uint32_t offset = 0;
    uint32_t size = 0;
};

constexpr size_t kMaxUniformMembers = 32;

struct UniformLayoutDesc {
    UniformMember const* members = nullptr;
    uint32_t memberCount = 0;
    uint32_t totalSize = 0;
};

} // namespace rendercore
} // namespace te
