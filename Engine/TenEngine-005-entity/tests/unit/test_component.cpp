#include <te/entity/Entity.hpp>
#include <te/entity/Component.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

int main() {
  te::scene::WorldRef world{};
  world.handle = reinterpret_cast<void*>(1u);
  te::entity::EntityId e = te::entity::CreateEntity(world);
  if (!te::entity::IsValid(e)) return 1;
  te::object::TypeId typeId = 1;
  te::entity::RegisterComponentType(typeId);
  te::entity::ComponentHandle h = te::entity::AddComponent(e, typeId);
  if (!te::entity::IsValid(h)) return 2;
  te::entity::ComponentHandle h2 = te::entity::GetComponent(e, typeId);
  if (h != h2) return 3;
  te::entity::RemoveComponent(e, h);
  if (te::entity::IsValid(h)) return 4;
  te::entity::DestroyEntity(e);
  return 0;
}
