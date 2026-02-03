// 009-RenderCore ResourceDesc Implementation (ABI: te/rendercore/resource_desc.hpp)

#include <te/rendercore/resource_desc.hpp>
#include <algorithm>

namespace te {
namespace rendercore {

VertexFormat CreateVertexFormat(VertexFormatDesc const& desc) {
  VertexFormat result{};

  if (desc.attributes == nullptr || desc.attributeCount == 0)
    return result;
  if (desc.attributeCount > kMaxVertexAttributes)
    return result;
  if (desc.stride == 0)
    return result;

  for (uint32_t i = 0; i < desc.attributeCount; ++i) {
    auto const& attr = desc.attributes[i];
    if (attr.format == VertexAttributeFormat::Unknown)
      return VertexFormat{};
    result.attributes[i] = attr;
  }

  result.attributeCount = desc.attributeCount;
  result.stride = desc.stride;
  return result;
}

IndexFormat CreateIndexFormat(IndexFormatDesc const& desc) {
  IndexFormat result{};
  if (desc.type == IndexType::Unknown)
    return result;
  result.type = desc.type;
  return result;
}

TextureDesc CreateTextureDesc(TextureDescParams const& params) {
  TextureDesc result{};
  if (params.width == 0 || params.height == 0)
    return result;
  if (params.format == TextureFormat::Unknown)
    return result;
  if (params.mipLevels == 0)
    return result;

  result.width = params.width;
  result.height = params.height;
  result.depth = params.depth == 0 ? 1 : params.depth;
  result.mipLevels = params.mipLevels;
  result.format = params.format;
  result.usage = params.usage;
  return result;
}

BufferDesc CreateBufferDesc(BufferDescParams const& params) {
  BufferDesc result{};
  if (params.size == 0)
    return result;

  result.size = params.size;
  result.usage = params.usage;
  result.alignment = params.alignment;
  return result;
}

}  // namespace rendercore
}  // namespace te
