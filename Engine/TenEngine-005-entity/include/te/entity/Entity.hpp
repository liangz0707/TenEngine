#ifndef TE_ENTITY_ENTITY_HPP
#define TE_ENTITY_ENTITY_HPP

#include <te/entity/EntityId.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

namespace te::entity {

using WorldRef = te::scene::WorldRef;
using NodeId = te::scene::NodeId;

EntityId CreateEntity(WorldRef world);
void DestroyEntity(EntityId entity);
NodeId GetSceneNode(EntityId entity);
void SetEnabled(EntityId entity, bool enabled);
bool IsValid(EntityId entity);

}

#endif
