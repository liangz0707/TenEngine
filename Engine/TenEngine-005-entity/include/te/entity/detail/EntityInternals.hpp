#ifndef TE_ENTITY_DETAIL_ENTITY_INTERNALS_HPP
#define TE_ENTITY_DETAIL_ENTITY_INTERNALS_HPP

#include <te/entity/EntityId.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>
#include <functional>

namespace te::entity {
namespace detail {

void ForEachEntityInWorld(te::scene::WorldRef world, std::function<void(EntityId)> fn);

}
}

#endif
