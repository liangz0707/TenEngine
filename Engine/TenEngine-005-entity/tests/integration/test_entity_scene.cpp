#include <te/entity/Entity.hpp>
#include <te/entity/Transform.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

int main() {
  te::scene::WorldRef world{};
  world.handle = reinterpret_cast<void*>(1u);
  te::entity::EntityId e = te::entity::CreateEntity(world);
  if (!te::entity::IsValid(e)) return 1;
  te::entity::NodeId node = te::entity::GetSceneNode(e);
  (void)node;
  te::entity::SetEnabled(e, true);
  te::scene::Transform local = te::entity::GetLocalTransform(e);
  te::entity::SetLocalTransform(e, local);
  te::entity::GetWorldTransform(e);
  te::entity::DestroyEntity(e);
  return 0;
}
