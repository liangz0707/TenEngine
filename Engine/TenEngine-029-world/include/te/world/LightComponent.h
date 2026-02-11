/**
 * @file LightComponent.h
 * @brief 029-World: Light component (point, directional, spot).
 * Contract: specs/_contracts/029-world-public-api.md, 005-entity ComponentUsageGuide.
 */

#ifndef TE_WORLD_LIGHT_COMPONENT_H
#define TE_WORLD_LIGHT_COMPONENT_H

#include <te/entity/Component.h>

namespace te {
namespace world {

enum class LightType : uint32_t {
  Point = 0,
  Directional,
  Spot,
};

/**
 * @brief Component holding light data for pipeline collection.
 */
struct LightComponent : public te::entity::Component {
  LightType type{LightType::Point};
  float color[3]{1.f, 1.f, 1.f};
  float intensity{1.f};
  float range{10.f};
  float direction[3]{0.f, -1.f, 0.f};  // normalized for directional/spot
  float spotAngle{0.5f};                // cone angle for spot

  LightComponent() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_LIGHT_COMPONENT_H
