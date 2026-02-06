#ifndef TE_ENTITY_COMPONENT_HPP
#define TE_ENTITY_COMPONENT_HPP

#include <te/entity/EntityId.hpp>
#include <te/entity/ComponentHandle.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

namespace te::entity {

using TypeId = te::object::TypeId;

bool RegisterComponentType(TypeId typeId);
ComponentHandle AddComponent(EntityId entity, TypeId typeId);
void RemoveComponent(EntityId entity, ComponentHandle handle);
ComponentHandle GetComponent(EntityId entity, TypeId typeId);
bool IsValid(ComponentHandle handle);

}

#endif
