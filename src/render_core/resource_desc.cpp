// 009-RenderCore ResourceDesc Implementation
// Contract: specs/_contracts/009-rendercore-public-api.md §2. ResourceDesc

#include "resource_desc.hpp"
#include <cstring>

namespace TenEngine::RenderCore {

// ============================================================================
// T005: CreateVertexFormat
// ============================================================================

VertexFormat CreateVertexFormat(VertexFormatDesc const& desc) {
    VertexFormat result{};

    // Validate input
    if (desc.attributes == nullptr || desc.attributeCount == 0) {
        return result; // Invalid: reject at call
    }
    if (desc.attributeCount > kMaxVertexAttributes) {
        return result; // Unsupported size: reject at call
    }
    if (desc.stride == 0) {
        return result; // Invalid stride: reject at call
    }

    // Copy attributes
    for (uint32_t i = 0; i < desc.attributeCount; ++i) {
        auto const& attr = desc.attributes[i];
        if (attr.format == VertexAttributeFormat::Unknown) {
            return VertexFormat{}; // Unsupported format: reject at call
        }
        result.attributes[i] = attr;
    }

    result.attributeCount = desc.attributeCount;
    result.stride = desc.stride;
    return result;
}

// ============================================================================
// T006: CreateIndexFormat
// ============================================================================

IndexFormat CreateIndexFormat(IndexFormatDesc const& desc) {
    IndexFormat result{};

    // Validate: only UInt16 and UInt32 supported
    if (desc.type == IndexType::Unknown) {
        return result; // Unsupported: reject at call
    }

    result.type = desc.type;
    return result;
}

// ============================================================================
// T007: CreateTextureDesc
// ============================================================================

TextureDesc CreateTextureDesc(TextureDescParams const& params) {
    TextureDesc result{};

    // Validate dimensions
    if (params.width == 0 || params.height == 0) {
        return result; // Invalid size: reject at call
    }

    // Validate format
    if (params.format == TextureFormat::Unknown) {
        return result; // Unsupported format: reject at call
    }

    // Validate mip levels (basic sanity check)
    if (params.mipLevels == 0) {
        return result; // Invalid: reject at call
    }

    result.width = params.width;
    result.height = params.height;
    result.depth = params.depth > 0 ? params.depth : 1;
    result.mipLevels = params.mipLevels;
    result.format = params.format;
    result.usage = params.usage;

    return result;
}

// ============================================================================
// T008: CreateBufferDesc
// ============================================================================

BufferDesc CreateBufferDesc(BufferDescParams const& params) {
    BufferDesc result{};

    // Validate size
    if (params.size == 0) {
        return result; // Invalid size: reject at call
    }

    result.size = params.size;
    result.usage = params.usage;
    result.alignment = params.alignment;

    return result;
}

} // namespace TenEngine::RenderCore
