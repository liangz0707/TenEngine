// 004-Scene: Hierarchy implementation. Single-use HierarchyIterator; Traverse, FindByName, FindByType, GetPath, GetId, SetActive, GetActive.

#include "scene/hierarchy.hpp"
#include "scene/scene_graph.hpp"
#include "scene/detail/scene_graph_detail.hpp"
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>

namespace te {
namespace scene {

namespace {

void CollectPreOrder(NodeId root, std::vector<NodeId>& out) {
  out.push_back(root);
  NodeId buf[64];
  size_t cnt = 64;
  GetChildren(root, buf, &cnt);
  for (size_t i = 0; i < cnt; ++i) CollectPreOrder(buf[i], out);
}

}  // namespace

struct HierarchyIterator::Impl {
  std::vector<NodeId> nodes;
  size_t index = 0;
};

HierarchyIterator::HierarchyIterator(Impl* impl) : impl_(impl) {}

HierarchyIterator::~HierarchyIterator() { delete impl_; impl_ = nullptr; }

bool HierarchyIterator::IsValid() const { return impl_ && impl_->index < impl_->nodes.size(); }
NodeId HierarchyIterator::GetId() const { return impl_ && impl_->index < impl_->nodes.size() ? impl_->nodes[impl_->index] : nullptr; }
void HierarchyIterator::Next() { if (impl_ && impl_->index < impl_->nodes.size()) ++impl_->index; }

HierarchyIterator Traverse(WorldRef world, NodeId root) {
  (void)world;
  std::vector<NodeId> nodes;
  if (root) CollectPreOrder(root, nodes);
  auto* impl = new HierarchyIterator::Impl;
  impl->nodes = std::move(nodes);
  return HierarchyIterator(impl);
}

HierarchyIterator FindByName(WorldRef world, NodeId root, char const* name) {
  (void)world;
  std::vector<NodeId> nodes;
  if (root && name) {
    std::vector<NodeId> all;
    CollectPreOrder(root, all);
    for (NodeId id : all) {
      if (detail::GetNodeName(id) && std::strcmp(detail::GetNodeName(id), name) == 0)
        nodes.push_back(id);
    }
  }
  auto* impl = new HierarchyIterator::Impl;
  impl->nodes = std::move(nodes);
  return HierarchyIterator(impl);
}

HierarchyIterator FindByType(WorldRef world, NodeId root, void* typeFilter) {
  (void)world;
  std::vector<NodeId> nodes;
  if (root) {
    std::vector<NodeId> all;
    CollectPreOrder(root, all);
    for (NodeId id : all) {
      if (detail::GetNodeType(id) == typeFilter)
        nodes.push_back(id);
    }
  }
  auto* impl = new HierarchyIterator::Impl;
  impl->nodes = std::move(nodes);
  return HierarchyIterator(impl);
}

void GetPath(NodeId node, char* pathBuffer, size_t bufferSize) {
  if (!pathBuffer || bufferSize == 0) return;
  pathBuffer[0] = '\0';
  if (!node) return;
  std::vector<NodeId> chain;
  for (NodeId n = node; n; n = GetParent(n))
    chain.push_back(n);
  size_t len = 0;
  for (size_t i = chain.size(); i > 0; --i) {
    const char* seg = detail::GetNodeName(chain[i - 1]);
    if (!seg || !seg[0]) seg = "?";
    size_t segLen = std::strlen(seg);
    if (len + 1 + segLen + 1 <= bufferSize) {
      if (len) pathBuffer[len++] = '/';
      std::memcpy(pathBuffer + len, seg, segLen + 1);
      len += segLen;
    } else
      break;
  }
  pathBuffer[len] = '\0';
}

NodeId GetId(HierarchyIterator const& it) {
  return it.GetId();
}

void SetActive(NodeId node, bool active) {
  detail::SetNodeActive(node, active);
}

bool GetActive(NodeId node) {
  return detail::GetNodeActive(node);
}

}  // namespace scene
}  // namespace te
