#include <te/entity/Component.hpp>
#include <te/entity/detail/ComponentInternals.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>
#include <map>
#include <vector>
#include <set>

namespace te::entity {

static std::set<TypeId> g_registeredComponentTypes;
struct ComponentRecord { EntityId entity; TypeId typeId; void* data = nullptr; };
static std::map<ComponentHandle, ComponentRecord> g_componentTable;
static std::map<EntityId, std::vector<ComponentHandle>> g_entityToHandles;
static uintptr_t g_nextHandle = 1;

bool RegisterComponentType(TypeId typeId) {
  g_registeredComponentTypes.insert(typeId);
  return true;
}

ComponentHandle AddComponent(EntityId entity, TypeId typeId) {
  if (entity.handle == nullptr || g_registeredComponentTypes.count(typeId) == 0) return ComponentHandle{};
  ComponentHandle h{reinterpret_cast<void*>(g_nextHandle++)};
  g_componentTable[h] = { entity, typeId, nullptr };
  g_entityToHandles[entity].push_back(h);
  return h;
}

void RemoveComponent(EntityId entity, ComponentHandle handle) {
  auto it = g_componentTable.find(handle);
  if (it == g_componentTable.end()) return;
  if (it->second.entity != entity) return;
  g_componentTable.erase(it);
  auto& vec = g_entityToHandles[entity];
  vec.erase(std::remove(vec.begin(), vec.end(), handle), vec.end());
}

ComponentHandle GetComponent(EntityId entity, TypeId typeId) {
  auto it = g_entityToHandles.find(entity);
  if (it == g_entityToHandles.end()) return ComponentHandle{};
  for (ComponentHandle h : it->second) {
    auto j = g_componentTable.find(h);
    if (j != g_componentTable.end() && j->second.typeId == typeId) return h;
  }
  return ComponentHandle{};
}

bool IsValid(ComponentHandle handle) {
  return handle.handle != nullptr && g_componentTable.find(handle) != g_componentTable.end();
}

namespace detail {

void DestroyComponentsForEntity(EntityId entity) {
  auto it = g_entityToHandles.find(entity);
  if (it == g_entityToHandles.end()) return;
  for (ComponentHandle h : it->second)
    g_componentTable.erase(h);
  g_entityToHandles.erase(it);
}

}
}
