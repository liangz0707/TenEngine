/**
 * @file MaterialResource.h
 * @brief IMaterialResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_MATERIAL_RESOURCE_H
#define TE_RESOURCE_MATERIAL_RESOURCE_H

#include <te/resource/Resource.h>

namespace te {
namespace resource {

/** Material resource view; implemented by 011; 013 returns IResource* then caller may cast. */
class IMaterialResource : public IResource {
 public:
  ~IMaterialResource() override = default;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_MATERIAL_RESOURCE_H
