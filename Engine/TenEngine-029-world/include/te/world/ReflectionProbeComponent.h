/**
 * @file ReflectionProbeComponent.h
 * @brief 029-World: Reflection probe (type, extent, resolution).
 * Contract: specs/_contracts/029-world-public-api.md, 005-entity ComponentUsageGuide.
 */

#ifndef TE_WORLD_REFLECTION_PROBE_COMPONENT_H
#define TE_WORLD_REFLECTION_PROBE_COMPONENT_H

#include <te/entity/Component.h>

namespace te {
namespace world {

enum class ReflectionProbeType : uint32_t {
  Box = 0,
  Sphere,
};

/**
 * @brief Component for reflection probe; pipeline collects for reflection pass.
 */
struct ReflectionProbeComponent : public te::entity::Component {
  ReflectionProbeType type{ReflectionProbeType::Sphere};
  float extent[3]{10.f, 10.f, 10.f};
  uint32_t resolution{128};

  ReflectionProbeComponent() = default;
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_REFLECTION_PROBE_COMPONENT_H
