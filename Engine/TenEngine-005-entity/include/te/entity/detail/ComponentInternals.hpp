#ifndef TE_ENTITY_DETAIL_COMPONENT_INTERNALS_HPP
#define TE_ENTITY_DETAIL_COMPONENT_INTERNALS_HPP

#include <te/entity/EntityId.hpp>

namespace te::entity {
namespace detail {

void DestroyComponentsForEntity(EntityId entity);

}
}

#endif
