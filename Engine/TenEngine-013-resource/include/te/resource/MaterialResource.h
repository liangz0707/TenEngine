// 013-Resource: IMaterialResource type view (te/resource/MaterialResource.h)
#pragma once

#include "te/resource/Resource.h"

namespace te {
namespace resource {

// Material resource view; cast from IResource* when GetResourceType() == ResourceType::Material.
// Concrete fields (shader, textures, params) defined by 011-Material.
struct IMaterialResource : IResource {};

} // namespace resource
} // namespace te
