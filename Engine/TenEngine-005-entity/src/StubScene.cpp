// Stub implementations when building without 004-scene. Exclude this file when linking with real te_scene.
#include <te/entity/detail/UpstreamFwd.hpp>
#include <cstdint>

namespace te {
namespace scene {

static NodeId s_nextNode{ reinterpret_cast<void*>(1u) };

NodeId CreateNode(WorldRef world) {
  (void)world;
  NodeId id = s_nextNode;
  s_nextNode.handle = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(s_nextNode.handle) + 1);
  return id;
}

void SetActive(NodeId node, bool active) { (void)node; (void)active; }
bool GetActive(NodeId node) { (void)node; return true; }
Transform GetLocalTransform(NodeId node) { (void)node; return Transform{}; }
void SetLocalTransform(NodeId node, Transform const& t) { (void)node; (void)t; }
Transform GetWorldTransform(NodeId node) { (void)node; return Transform{}; }

}
}
