#include <te/entity/ComponentQuery.hpp>
#include <te/entity/Component.hpp>
#include <te/entity/Entity.hpp>
#include <te/entity/detail/EntityInternals.hpp>
#include <utility>

namespace te::entity {

ComponentQuery ComponentQuery::ByComponentType(WorldRef world, TypeId componentType) {
  std::vector<std::pair<EntityId, ComponentHandle>> snap;
  detail::ForEachEntityInWorld(world, [&snap, componentType](EntityId e) {
    ComponentHandle h = GetComponent(e, componentType);
    if (IsValid(h))
      snap.push_back({ e, h });
  });
  ComponentQuery q;
  q.snapshot_ = std::move(snap);
  return q;
}

ComponentQuery::Iterator ComponentQuery::Begin() {
  return Iterator(&snapshot_, 0);
}

ComponentQuery::Iterator ComponentQuery::End() const {
  return Iterator(&snapshot_, snapshot_.size());
}

bool ComponentQuery::Iterator::Next() {
  if (!snapshot || index >= snapshot->size()) return false;
  ++index;
  return index <= snapshot->size();
}

EntityId ComponentQuery::Iterator::GetEntityId() const {
  if (!snapshot || index == 0 || index > snapshot->size()) return EntityId{};
  return (*snapshot)[index - 1].first;
}

ComponentHandle ComponentQuery::Iterator::GetComponentHandle() const {
  if (!snapshot || index == 0 || index > snapshot->size()) return ComponentHandle{};
  return (*snapshot)[index - 1].second;
}

}
