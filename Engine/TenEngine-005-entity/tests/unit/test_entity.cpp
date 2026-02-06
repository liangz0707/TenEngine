#include <te/entity/Entity.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

int main() {
  te::scene::WorldRef world{};
  world.handle = reinterpret_cast<void*>(1u);
  te::entity::EntityId e = te::entity::CreateEntity(world);
  if (!te::entity::IsValid(e)) return 1;
  te::entity::NodeId node = te::entity::GetSceneNode(e);
  (void)node;
  te::entity::SetEnabled(e, false);
  te::entity::SetEnabled(e, true);
  te::entity::DestroyEntity(e);
  if (te::entity::IsValid(e)) return 2;
  return 0;
}
