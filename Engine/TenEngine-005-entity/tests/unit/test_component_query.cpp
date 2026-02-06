#include <te/entity/Entity.hpp>
#include <te/entity/Component.hpp>
#include <te/entity/ComponentQuery.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

int main() {
  te::scene::WorldRef world{};
  world.handle = reinterpret_cast<void*>(1u);
  te::entity::EntityId e = te::entity::CreateEntity(world);
  te::object::TypeId typeId = 1;
  te::entity::RegisterComponentType(typeId);
  te::entity::AddComponent(e, typeId);
  auto q = te::entity::ComponentQuery::ByComponentType(world, typeId);
  auto it = q.Begin();
  int count = 0;
  while (it.Next()) {
    (void)it.GetEntityId();
    (void)it.GetComponentHandle();
    ++count;
  }
  te::entity::DestroyEntity(e);
  return (count >= 1) ? 0 : 1;
}
