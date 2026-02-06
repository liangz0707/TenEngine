/** @file resource_desc.hpp
 *  009-RenderCore ABI: VertexFormat, IndexFormat, TextureDesc, BufferDesc,
 *  CreateVertexFormat, CreateIndexFormat, CreateTextureDesc, CreateBufferDesc.
 */
#pragma once

#include <te/rendercore/types.hpp>
#include <cstdint>

namespace te {
namespace rendercore {

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

enum class TextureFormat : uint16_t {
  Unknown = 0,
  R8_UNorm, RG8_UNorm, RGBA8_UNorm, RGBA8_SRGB, BGRA8_UNorm,
  R16_Float, RG16_Float, RGBA16_Float,
  R32_Float, RG32_Float, RGBA32_Float,
  Depth16, Depth24_Stencil8, Depth32_Float
};

enum class TextureUsage : uint32_t {
  None = 0,
  Sampled = 1u << 0, Storage = 1u << 1, RenderTarget = 1u << 2,
  DepthStencil = 1u << 3, TransferSrc = 1u << 4, TransferDst = 1u << 5
};

inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
  return static_cast<TextureUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

struct TextureDesc {
  uint32_t width = 0, height = 0, depth = 1, mipLevels = 1;
  TextureFormat format = TextureFormat::Unknown;
  TextureUsage usage = TextureUsage::None;
  bool IsValid() const { return width > 0 && height > 0 && format != TextureFormat::Unknown; }
};

struct TextureDescParams {
  uint32_t width = 0, height = 0, depth = 1, mipLevels = 1;
  TextureFormat format = TextureFormat::Unknown;
  TextureUsage usage = TextureUsage::None;
};

enum class BufferUsage : uint32_t {
  None = 0,
  Vertex = 1u << 0, Index = 1u << 1, Uniform = 1u << 2,
  Storage = 1u << 3, TransferSrc = 1u << 4, TransferDst = 1u << 5
};

inline BufferUsage operator|(BufferUsage a, BufferUsage b) {
  return static_cast<BufferUsage>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
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

VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);
IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);
TextureDesc CreateTextureDesc(TextureDescParams const& params);
BufferDesc CreateBufferDesc(BufferDescParams const& params);

}  // namespace rendercore
}  // namespace te
