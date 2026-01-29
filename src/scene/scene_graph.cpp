// 004-Scene: SceneGraph implementation. Contract-only API: CreateNode, SetParent, GetParent, GetChildren, Local/WorldTransform, SetDirty, UpdateTransforms.

#include "scene/scene_graph.hpp"
#include "scene/detail/scene_graph_detail.hpp"
#include "scene/world.hpp"
#include <vector>
#include <memory>
#include <map>
#include <algorithm>
#include <cstring>

namespace ten {
namespace scene {

namespace {

struct NodeImpl {
  NodeImpl* parent = nullptr;
  std::vector<NodeImpl*> children;
  Transform localTransform;
  Transform worldTransform;
  bool dirty = true;
  bool active = true;
  char name[64] = {};
  void* typeTag = nullptr;
};

std::vector<std::unique_ptr<NodeImpl>> g_allNodes;
std::map<WorldRef, size_t> g_worldToIndex;
std::vector<std::vector<NodeImpl*>> g_worldNodes;
std::vector<NodeImpl*> g_worldRoot;

NodeImpl* Impl(NodeId id) { return static_cast<NodeImpl*>(id); }
NodeId ToId(NodeImpl* n) { return static_cast<void*>(n); }

bool WouldCycle(NodeImpl* node, NodeImpl* newParent) {
  if (!newParent || node == newParent) return true;
  for (NodeImpl* p = newParent; p; p = p->parent)
    if (p == node) return true;
  return false;
}

void UpdateWorldTransformRecursive(NodeImpl* n, const Transform* parentWorld) {
  if (!n) return;
  if (parentWorld) {
    n->worldTransform.position[0] = parentWorld->position[0] + n->localTransform.position[0];
    n->worldTransform.position[1] = parentWorld->position[1] + n->localTransform.position[1];
    n->worldTransform.position[2] = parentWorld->position[2] + n->localTransform.position[2];
    n->worldTransform.rotation[0] = n->localTransform.rotation[0];
    n->worldTransform.rotation[1] = n->localTransform.rotation[1];
    n->worldTransform.rotation[2] = n->localTransform.rotation[2];
    n->worldTransform.rotation[3] = n->localTransform.rotation[3];
    n->worldTransform.scale[0] = parentWorld->scale[0] * n->localTransform.scale[0];
    n->worldTransform.scale[1] = parentWorld->scale[1] * n->localTransform.scale[1];
    n->worldTransform.scale[2] = parentWorld->scale[2] * n->localTransform.scale[2];
  } else {
    n->worldTransform = n->localTransform;
  }
  n->dirty = false;
  for (NodeImpl* c : n->children)
    UpdateWorldTransformRecursive(c, &n->worldTransform);
}

}  // namespace

NodeId CreateNode(WorldRef world) {
  auto node = std::make_unique<NodeImpl>();
  NodeImpl* raw = node.get();
  g_allNodes.push_back(std::move(node));
  if (world) {
    size_t wi;
    auto it = g_worldToIndex.find(world);
    if (it == g_worldToIndex.end()) {
      wi = g_worldNodes.size();
      g_worldToIndex[world] = wi;
      g_worldNodes.emplace_back();
      g_worldRoot.push_back(nullptr);
    } else {
      wi = it->second;
    }
    if (!g_worldRoot[wi]) {
      g_worldRoot[wi] = raw;
      raw->parent = nullptr;
    } else {
      raw->parent = g_worldRoot[wi];
      g_worldRoot[wi]->children.push_back(raw);
    }
    g_worldNodes[wi].push_back(raw);
  }
  return ToId(raw);
}

bool SetParent(NodeId node, NodeId parent) {
  NodeImpl* n = Impl(node);
  NodeImpl* p = Impl(parent);
  if (!n) return false;
  if (p && (p == n || WouldCycle(n, p))) return false;
  if (n->parent) {
    auto& siblings = n->parent->children;
    siblings.erase(std::remove(siblings.begin(), siblings.end(), n), siblings.end());
  }
  n->parent = p;
  if (p) p->children.push_back(n);
  n->dirty = true;
  return true;
}

NodeId GetParent(NodeId node) {
  NodeImpl* n = Impl(node);
  return n && n->parent ? ToId(n->parent) : nullptr;
}

void GetChildren(NodeId node, NodeId* children, size_t* count) {
  NodeImpl* n = Impl(node);
  if (!n || !count) { if (count) *count = 0; return; }
  size_t cap = *count;
  size_t nchild = n->children.size();
  size_t write = (cap < nchild) ? cap : nchild;
  if (children)
    for (size_t i = 0; i < write; ++i) children[i] = ToId(n->children[i]);
  *count = nchild;
}

Transform GetLocalTransform(NodeId node) {
  NodeImpl* n = Impl(node);
  return n ? n->localTransform : Transform{};
}

void SetLocalTransform(NodeId node, Transform const& t) {
  NodeImpl* n = Impl(node);
  if (n) { n->localTransform = t; n->dirty = true; }
}

Transform GetWorldTransform(NodeId node) {
  NodeImpl* n = Impl(node);
  return n ? n->worldTransform : Transform{};
}

void SetDirty(NodeId node) {
  NodeImpl* n = Impl(node);
  if (n) n->dirty = true;
}

void UpdateTransforms(WorldRef world) {
  ApplyPendingActiveWorld();
  if (!world) return;
  auto it = g_worldToIndex.find(world);
  if (it == g_worldToIndex.end()) return;
  size_t wi = it->second;
  if (wi >= g_worldRoot.size()) return;
  NodeImpl* root = g_worldRoot[wi];
  if (root) UpdateWorldTransformRecursive(root, nullptr);
}

namespace detail {

const char* GetNodeName(NodeId node) {
  NodeImpl* n = Impl(node);
  return n ? n->name : "";
}

void SetNodeName(NodeId node, char const* name) {
  NodeImpl* n = Impl(node);
  if (n && name) {
    std::strncpy(n->name, name, sizeof(n->name) - 1);
    n->name[sizeof(n->name) - 1] = '\0';
  }
}

void* GetNodeType(NodeId node) {
  NodeImpl* n = Impl(node);
  return n ? n->typeTag : nullptr;
}

void SetNodeType(NodeId node, void* typeTag) {
  NodeImpl* n = Impl(node);
  if (n) n->typeTag = typeTag;
}

bool GetNodeActive(NodeId node) {
  NodeImpl* n = Impl(node);
  return n ? n->active : true;
}

void SetNodeActive(NodeId node, bool active) {
  NodeImpl* n = Impl(node);
  if (n) n->active = active;
}

}  // namespace detail

}  // namespace scene
}  // namespace ten
