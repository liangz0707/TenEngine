#include <te/entity/Entity.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>
#include <te/entity/detail/ComponentInternals.hpp>
#include <te/entity/detail/EntityInternals.hpp>
#include <map>
#include <functional>

namespace te::entity {

struct EntityEntry {
  te::scene::NodeId node;
  te::scene::WorldRef world{};
  bool enabled = true;
};
static std::map<EntityId, EntityEntry> g_entityTable;

EntityId CreateEntity(WorldRef world) {
  te::scene::NodeId node = te::scene::CreateNode(world);
  if (node.handle == nullptr) return EntityId{};
  EntityId id{node.handle};
  g_entityTable[id] = { node, world, true };
  return id;
}

void DestroyEntity(EntityId entity) {
  auto it = g_entityTable.find(entity);
  if (it == g_entityTable.end()) return;
  detail::DestroyComponentsForEntity(entity);
  g_entityTable.erase(it);
}

NodeId GetSceneNode(EntityId entity) {
  auto it = g_entityTable.find(entity);
  if (it == g_entityTable.end()) return {};
  return it->second.node;
}

void SetEnabled(EntityId entity, bool enabled) {
  auto it = g_entityTable.find(entity);
  if (it == g_entityTable.end()) return;
  it->second.enabled = enabled;
  te::scene::SetActive(it->second.node, enabled);
}

bool IsValid(EntityId entity) {
  return entity.handle != nullptr && g_entityTable.find(entity) != g_entityTable.end();
}

namespace detail {

void ForEachEntityInWorld(te::scene::WorldRef world, std::function<void(EntityId)> fn) {
  for (auto const& kv : g_entityTable)
    if (kv.second.world.handle == world.handle)
      fn(kv.first);
}

}
}
