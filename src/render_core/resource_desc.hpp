#pragma once
// 009-RenderCore ResourceDesc API
// Contract: specs/_contracts/009-rendercore-public-api.md §2. ResourceDesc

#include "types.hpp"

namespace TenEngine::RenderCore {

/// Create VertexFormat from descriptor.
/// Returns invalid format if unsupported (reject at call).
VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);

/// Create IndexFormat from descriptor.
/// Returns invalid format if unsupported (reject at call).
IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);

/// Create TextureDesc from params.
/// Returns invalid desc if unsupported (reject at call).
TextureDesc CreateTextureDesc(TextureDescParams const& params);

/// Create BufferDesc from params.
/// Returns invalid desc if unsupported (reject at call).
BufferDesc CreateBufferDesc(BufferDescParams const& params);

} // namespace TenEngine::RenderCore
