// 004-Scene: SceneGraph API (contract). Node, Parent/Children, Local/WorldTransform, SetDirty, UpdateTransforms.

#ifndef TENENGINE_SCENE_SCENE_GRAPH_HPP
#define TENENGINE_SCENE_SCENE_GRAPH_HPP

#include "scene/types.hpp"
#include <cstddef>

namespace te {
namespace scene {

NodeId CreateNode(WorldRef world);
bool SetParent(NodeId node, NodeId parent);
NodeId GetParent(NodeId node);
void GetChildren(NodeId node, NodeId* children, size_t* count);

Transform GetLocalTransform(NodeId node);
void SetLocalTransform(NodeId node, Transform const& t);
Transform GetWorldTransform(NodeId node);

void SetDirty(NodeId node);
void UpdateTransforms(WorldRef world);

}  // namespace scene
}  // namespace te

#endif
