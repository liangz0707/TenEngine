#ifndef TE_ENTITY_COMPONENT_QUERY_HPP
#define TE_ENTITY_COMPONENT_QUERY_HPP

#include <te/entity/EntityId.hpp>
#include <te/entity/ComponentHandle.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>
#include <vector>
#include <cstddef>

namespace te::entity {

using WorldRef = te::scene::WorldRef;
using TypeId = te::object::TypeId;

struct ComponentQuery {
  struct Iterator {
    bool Next();
    EntityId GetEntityId() const;
    ComponentHandle GetComponentHandle() const;

   private:
    using Snapshot = std::vector<std::pair<EntityId, ComponentHandle>>;
    Snapshot const* snapshot = nullptr;
    std::size_t index = 0;
    Iterator(Snapshot const* s, std::size_t i) : snapshot(s), index(i) {}
    friend struct ComponentQuery;
  };
  static ComponentQuery ByComponentType(WorldRef world, TypeId componentType);
  Iterator Begin();
  Iterator End() const;

 private:
  std::vector<std::pair<EntityId, ComponentHandle>> snapshot_;
};

}

#endif
