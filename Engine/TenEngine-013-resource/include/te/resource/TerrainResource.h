/**
 * @file TerrainResource.h
 * @brief ITerrainResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_TERRAIN_RESOURCE_H
#define TE_RESOURCE_TERRAIN_RESOURCE_H

#include <te/resource/Resource.h>

namespace te {
namespace resource {

/** Terrain resource view; heightmap etc.; 013 returns IResource* then caller may cast. */
class ITerrainResource : public IResource {
 public:
  ~ITerrainResource() override = default;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_TERRAIN_RESOURCE_H
