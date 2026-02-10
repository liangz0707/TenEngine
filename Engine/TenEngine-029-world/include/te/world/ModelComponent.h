/**
 * @file ModelComponent.h
 * @brief 029-World: Model component (ResourceId reference to model resource).
 * Contract: specs/_contracts/029-world-public-api.md, 005-entity ComponentUsageGuide.
 */

#ifndef TE_WORLD_MODEL_COMPONENT_H
#define TE_WORLD_MODEL_COMPONENT_H

#include <te/entity/Component.h>
#include <te/resource/ResourceId.h>

namespace te {
namespace world {

/**
 * @brief Component holding a reference to a model resource (029 owns this type).
 * Pipeline resolves modelResourceId via 013 LoadSync/GetCached to IModelResource* for rendering.
 */
struct ModelComponent : public te::entity::Component {
    te::resource::ResourceId modelResourceId;

    ModelComponent() : modelResourceId{} {}
    explicit ModelComponent(te::resource::ResourceId const& id) : modelResourceId(id) {}
};

}  // namespace world
}  // namespace te

#endif  // TE_WORLD_MODEL_COMPONENT_H
