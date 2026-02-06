// 013-Resource: ITextureResource type view (te/resource/TextureResource.h)
#pragma once

#include "te/resource/Resource.h"

namespace te {
namespace resource {

// Texture resource view; cast from IResource* when GetResourceType() == ResourceType::Texture.
// Concrete fields (width, height, format, GPU handle) defined by 028-Texture.
struct ITextureResource : IResource {};

} // namespace resource
} // namespace te
