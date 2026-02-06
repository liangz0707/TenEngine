#include <te/entity/Entity.hpp>
#include <te/entity/Transform.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

int main() {
  te::scene::WorldRef world{};
  world.handle = reinterpret_cast<void*>(1u);
  te::entity::EntityId e = te::entity::CreateEntity(world);
  if (!te::entity::IsValid(e)) return 1;
  te::scene::Transform t = te::entity::GetLocalTransform(e);
  (void)t;
  te::entity::SetLocalTransform(e, t);
  te::scene::Transform w = te::entity::GetWorldTransform(e);
  (void)w;
  te::entity::DestroyEntity(e);
  return 0;
}
