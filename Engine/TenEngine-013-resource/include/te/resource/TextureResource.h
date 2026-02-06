/**
 * @file TextureResource.h
 * @brief ITextureResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_TEXTURE_RESOURCE_H
#define TE_RESOURCE_TEXTURE_RESOURCE_H

#include <te/resource/Resource.h>

namespace te {
namespace resource {

/** Texture resource view; implemented by 028; 013 returns IResource* then caller may cast. */
class ITextureResource : public IResource {
 public:
  ~ITextureResource() override = default;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_TEXTURE_RESOURCE_H
