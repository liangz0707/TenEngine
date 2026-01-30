#pragma once
// 009-RenderCore Contract Types
// Only types declared in specs/_contracts/009-rendercore-public-api.md are exposed.
// No RHI internals; uses only 001-Core and 008-RHI contract types.

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace TenEngine::RenderCore {

// ============================================================================
// Error / Result (T004)
// ============================================================================

/// Result code for contract API calls (reject-at-call semantics).
enum class ResultCode : uint32_t {
    Success = 0,
    InvalidHandle,
    UnsupportedFormat,
    UnsupportedSize,
    ValidationFailed,
    RingBufferExhausted,  // Caller should wait/retry (Block)
    Unknown
};

// ============================================================================
// Forward declarations for handles
// ============================================================================

struct UniformLayoutImpl;
struct UniformBufferImpl;

// ============================================================================
// Opaque Handles (contract types)
// ============================================================================

/// Opaque handle for Uniform Buffer layout; lifecycle: defined until unload.
struct UniformLayout {
    UniformLayoutImpl* impl = nullptr;
    bool IsValid() const { return impl != nullptr; }
};

/// Opaque handle for Uniform buffer; lifecycle: created until explicit release.
struct UniformBufferHandle {
    UniformBufferImpl* impl = nullptr;
    bool IsValid() const { return impl != nullptr; }
};

// ============================================================================
// Resource Lifetime (PassProtocol)
// ============================================================================

/// Resource lifetime for PassResourceDecl; aligns with PipelineCore RDG protocol.
enum class ResourceLifetime : uint8_t {
    Transient,   // Single pass-graph build
    Persistent,  // Across frames
    External     // Managed externally
};

// ============================================================================
// PassHandle / ResourceHandle (for PassProtocol)
// ============================================================================

/// Pass identifier/handle for DeclareRead/DeclareWrite; single pass-graph build or managed by PipelineCore.
struct PassHandle {
    uint64_t id = 0;
    bool IsValid() const { return id != 0; }
};

/// Resource handle for declaring pass read/write; lifecycle with resource or managed by PipelineCore.
struct ResourceHandle {
    uint64_t id = 0;
    bool IsValid() const { return id != 0; }
};

/// Bind slot for UniformBuffer::Bind; aligns with Shader module and RHI binding.
struct BindSlot {
    uint32_t set = 0;
    uint32_t binding = 0;
};

// ============================================================================
// PassResourceDecl (PassProtocol output)
// ============================================================================

/// Pass input/output resource declaration; lifecycle: single pass-graph build.
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

/// Vertex attribute format (subset of common formats for contract).
enum class VertexAttributeFormat : uint8_t {
    Float,
    Float2,
    Float3,
    Float4,
    Int,
    Int2,
    Int3,
    Int4,
    UInt,
    UInt2,
    UInt3,
    UInt4,
    Unknown
};

/// Single vertex attribute descriptor.
struct VertexAttribute {
    uint32_t location = 0;
    VertexAttributeFormat format = VertexAttributeFormat::Unknown;
    uint32_t offset = 0;
};

/// Max vertex attributes per format.
constexpr size_t kMaxVertexAttributes = 16;

/// VertexFormat descriptor; lifecycle: defined until unload.
struct VertexFormat {
    VertexAttribute attributes[kMaxVertexAttributes];
    uint32_t attributeCount = 0;
    uint32_t stride = 0;

    bool IsValid() const { return attributeCount > 0 && stride > 0; }
};

/// VertexFormatDesc for CreateVertexFormat input.
struct VertexFormatDesc {
    VertexAttribute const* attributes = nullptr;
    uint32_t attributeCount = 0;
    uint32_t stride = 0;
};

/// Index type enum.
enum class IndexType : uint8_t {
    UInt16,
    UInt32,
    Unknown
};

/// IndexFormat descriptor; lifecycle: defined until unload.
struct IndexFormat {
    IndexType type = IndexType::Unknown;
    bool IsValid() const { return type != IndexType::Unknown; }
};

/// IndexFormatDesc for CreateIndexFormat input.
struct IndexFormatDesc {
    IndexType type = IndexType::Unknown;
};

// ============================================================================
// TextureDesc / BufferDesc (ResourceDesc)
// ============================================================================

/// Texture format (subset for contract).
enum class TextureFormat : uint16_t {
    Unknown = 0,
    R8_UNorm,
    RG8_UNorm,
    RGBA8_UNorm,
    RGBA8_SRGB,
    BGRA8_UNorm,
    R16_Float,
    RG16_Float,
    RGBA16_Float,
    R32_Float,
    RG32_Float,
    RGBA32_Float,
    Depth16,
    Depth24_Stencil8,
    Depth32_Float
};

/// Texture usage flags.
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

/// Texture descriptor; lifecycle: managed by caller.
struct TextureDesc {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::Unknown;
    TextureUsage usage = TextureUsage::None;

    bool IsValid() const { return width > 0 && height > 0 && format != TextureFormat::Unknown; }
};

/// TextureDescParams for CreateTextureDesc input.
struct TextureDescParams {
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 1;
    uint32_t mipLevels = 1;
    TextureFormat format = TextureFormat::Unknown;
    TextureUsage usage = TextureUsage::None;
};

/// Buffer usage flags.
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

/// Buffer descriptor; lifecycle: managed by caller.
struct BufferDesc {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    uint32_t alignment = 0;

    bool IsValid() const { return size > 0; }
};

/// BufferDescParams for CreateBufferDesc input.
struct BufferDescParams {
    uint64_t size = 0;
    BufferUsage usage = BufferUsage::None;
    uint32_t alignment = 0;
};

// ============================================================================
// UniformLayoutDesc / Member (ShaderParams)
// ============================================================================

/// Uniform member type (subset).
enum class UniformMemberType : uint8_t {
    Float,
    Float2,
    Float3,
    Float4,
    Mat3,
    Mat4,
    Int,
    Int2,
    Int3,
    Int4,
    Unknown
};

/// Single uniform member descriptor.
struct UniformMember {
    char name[64] = {};
    UniformMemberType type = UniformMemberType::Unknown;
    uint32_t offset = 0;  // Byte offset (0 = auto-calculate)
    uint32_t size = 0;    // Byte size (0 = auto from type)
};

/// Max members per layout.
constexpr size_t kMaxUniformMembers = 32;

/// UniformLayoutDesc for DefineLayout input.
struct UniformLayoutDesc {
    UniformMember const* members = nullptr;
    uint32_t memberCount = 0;
    uint32_t totalSize = 0;  // 0 = auto-calculate
};

} // namespace TenEngine::RenderCore
