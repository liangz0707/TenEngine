/**
 * @file DecalComponent.h
 * @brief 029-World: Decal (projection, texture, blend).
 * Contract: specs/_contracts/029-world-public-api.md, 005-entity ComponentUsageGuide.
 */

#ifndef TE_WORLD_DECAL_COMPONENT_H
#define TE_WORLD_DECAL_COMPONENT_H

#include <te/entity/Component.h>
#include <te/resource/ResourceId.h>

namespace te {
namespace world {

/**
 * @brief Component for decal; pipeline collects for decal pass.
 */
struct DecalComponent : public te::entity::Component {
  te::resource::ResourceId albedoTextureId;
  float size[3]{2.f, 2.f, 2.f};
  float blend{1.f};

  DecalComponent() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_DECAL_COMPONENT_H
