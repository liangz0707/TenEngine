#pragma once
// 009-RenderCore ResourceDesc API (te::rendercore)
// ABI: specs/_contracts/009-rendercore-ABI.md

#include <te/rendercore/types.hpp>

namespace te {
namespace rendercore {

VertexFormat CreateVertexFormat(VertexFormatDesc const& desc);
IndexFormat CreateIndexFormat(IndexFormatDesc const& desc);
TextureDesc CreateTextureDesc(TextureDescParams const& params);
BufferDesc CreateBufferDesc(BufferDescParams const& params);

} // namespace rendercore
} // namespace te
