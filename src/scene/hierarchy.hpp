// 004-Scene: HierarchyIterator (contract: single-use, invalid after traverse ends).
// Traverse, FindByName, FindByType return this; caller must not use after traverse ends.

#ifndef TENENGINE_SCENE_HIERARCHY_HPP
#define TENENGINE_SCENE_HIERARCHY_HPP

#include "scene/types.hpp"

namespace ten {
namespace scene {

// Single-use iterator for hierarchy traverse / find. Move-only; invalid after traverse ends.
class HierarchyIterator {
 public:
  HierarchyIterator() = default;
  ~HierarchyIterator();
  HierarchyIterator(HierarchyIterator&& other) noexcept : impl_(other.impl_) { other.impl_ = nullptr; }
  HierarchyIterator& operator=(HierarchyIterator&& other) noexcept {
    if (this != &other) { delete impl_; impl_ = other.impl_; other.impl_ = nullptr; }
    return *this;
  }
  HierarchyIterator(const HierarchyIterator&) = delete;
  HierarchyIterator& operator=(const HierarchyIterator&) = delete;

  bool IsValid() const;
  NodeId GetId() const;
  void Next();

 private:
  struct Impl;
  Impl* impl_ = nullptr;
  friend HierarchyIterator Traverse(WorldRef world, NodeId root);
  friend HierarchyIterator FindByName(WorldRef world, NodeId root, char const* name);
  friend HierarchyIterator FindByType(WorldRef world, NodeId root, void* typeFilter);
  explicit HierarchyIterator(Impl* impl);
};

HierarchyIterator Traverse(WorldRef world, NodeId root);
HierarchyIterator FindByName(WorldRef world, NodeId root, char const* name);
HierarchyIterator FindByType(WorldRef world, NodeId root, void* typeFilter);
void GetPath(NodeId node, char* pathBuffer, size_t bufferSize);
NodeId GetId(HierarchyIterator const& it);
void SetActive(NodeId node, bool active);
bool GetActive(NodeId node);

}  // namespace scene
}  // namespace ten

#endif
