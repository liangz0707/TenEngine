#include <te/entity/Transform.hpp>
#include <te/entity/Entity.hpp>
#include <te/entity/detail/UpstreamFwd.hpp>

namespace te::entity {

Transform GetLocalTransform(EntityId entity) {
  NodeId node = GetSceneNode(entity);
  return te::scene::GetLocalTransform(node);
}

void SetLocalTransform(EntityId entity, Transform const& t) {
  NodeId node = GetSceneNode(entity);
  te::scene::SetLocalTransform(node, t);
}

Transform GetWorldTransform(EntityId entity) {
  NodeId node = GetSceneNode(entity);
  return te::scene::GetWorldTransform(node);
}

}
