/**
 * @file EffectResource.h
 * @brief IEffectResource view (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_EFFECT_RESOURCE_H
#define TE_RESOURCE_EFFECT_RESOURCE_H

#include <te/resource/Resource.h>

namespace te {
namespace resource {

/** Effect resource view; particles/VFX; 013 returns IResource* then caller may cast. */
class IEffectResource : public IResource {
 public:
  ~IEffectResource() override = default;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_EFFECT_RESOURCE_H
