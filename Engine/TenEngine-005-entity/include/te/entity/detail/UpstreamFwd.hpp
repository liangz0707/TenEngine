#ifndef TE_ENTITY_DETAIL_UPSTREAM_FWD_HPP
#define TE_ENTITY_DETAIL_UPSTREAM_FWD_HPP

// Minimal types/declarations for upstream contract (004-Scene, 002-Object).
// When building with real deps, link te_scene/te_object; they provide the definitions.
namespace te {
namespace scene {
struct WorldRef { void* handle = nullptr; };
struct NodeId { void* handle = nullptr; };
struct Transform { float position[3]; float rotation[4]; float scale[3]; };
NodeId CreateNode(WorldRef world);
void SetActive(NodeId node, bool active);
bool GetActive(NodeId node);
Transform GetLocalTransform(NodeId node);
void SetLocalTransform(NodeId node, Transform const& t);
Transform GetWorldTransform(NodeId node);
}
namespace object {
using TypeId = unsigned;
}
}

#endif
