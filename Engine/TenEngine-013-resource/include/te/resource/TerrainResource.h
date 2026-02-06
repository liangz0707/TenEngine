// 013-Resource: ITerrainResource type view (te/resource/TerrainResource.h)
#pragma once

#include "te/resource/Resource.h"

namespace te {
namespace resource {

// Terrain resource view; cast from IResource* when GetResourceType() == ResourceType::Terrain.
struct ITerrainResource : IResource {};

} // namespace resource
} // namespace te
