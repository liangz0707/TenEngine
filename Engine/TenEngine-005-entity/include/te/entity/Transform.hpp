#ifndef TE_ENTITY_TRANSFORM_HPP
#define TE_ENTITY_TRANSFORM_HPP

#include <te/entity/Entity.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

namespace te::entity {

using Transform = te::scene::Transform;

Transform GetLocalTransform(EntityId entity);
void SetLocalTransform(EntityId entity, Transform const& t);
Transform GetWorldTransform(EntityId entity);

}

#endif
