// 004-Scene: Unit tests for hierarchy (Traverse, FindByName, FindByType, GetPath, GetId, SetActive, GetActive, iterator single-use).

#include "scene/hierarchy.hpp"
#include "scene/scene_graph.hpp"
#include "scene/world.hpp"
#include "scene/detail/scene_graph_detail.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <utility>

using namespace te::scene;

static void test_traverse_order() {
  WorldRef w = GetCurrentWorld();
  assert(w != nullptr);
  NodeId root = CreateNode(w);
  NodeId c1 = CreateNode(w);
  NodeId c2 = CreateNode(w);
  SetParent(c1, root);
  SetParent(c2, root);
  HierarchyIterator it = Traverse(w, root);
  assert(it.IsValid());
  assert(GetId(it) == root);
  it.Next();
  assert(it.IsValid());
  assert(GetId(it) == c1 || GetId(it) == c2);
  it.Next();
  assert(it.IsValid());
  it.Next();
  assert(!it.IsValid());
}

static void test_find_by_name() {
  WorldRef w = GetCurrentWorld();
  NodeId root = CreateNode(w);
  NodeId child = CreateNode(w);
  SetParent(child, root);
  detail::SetNodeName(root, "root");
  detail::SetNodeName(child, "child");
  HierarchyIterator it = FindByName(w, root, "child");
  assert(it.IsValid());
  assert(GetId(it) == child);
  it.Next();
  assert(!it.IsValid());
  HierarchyIterator it2 = FindByName(w, root, "nonexistent");
  assert(!it2.IsValid());
}

static void test_find_by_type() {
  WorldRef w = GetCurrentWorld();
  NodeId root = CreateNode(w);
  NodeId a = CreateNode(w);
  NodeId b = CreateNode(w);
  SetParent(a, root);
  SetParent(b, root);
  void* typeA = reinterpret_cast<void*>(1);
  void* typeB = reinterpret_cast<void*>(2);
  detail::SetNodeType(root, typeA);
  detail::SetNodeType(a, typeA);
  detail::SetNodeType(b, typeB);
  HierarchyIterator it = FindByType(w, root, typeA);
  assert(it.IsValid());
  assert(GetId(it) == root || GetId(it) == a);
  size_t count = 0;
  while (it.IsValid()) { ++count; it.Next(); }
  assert(count == 2);
  HierarchyIterator itB = FindByType(w, root, typeB);
  assert(itB.IsValid());
  assert(GetId(itB) == b);
}

static void test_get_path() {
  WorldRef w = GetCurrentWorld();
  NodeId root = CreateNode(w);
  NodeId child = CreateNode(w);
  NodeId grand = CreateNode(w);
  SetParent(child, root);
  SetParent(grand, child);
  detail::SetNodeName(root, "R");
  detail::SetNodeName(child, "C");
  detail::SetNodeName(grand, "G");
  char path[256];
  GetPath(grand, path, sizeof(path));
  assert(std::strstr(path, "R") != nullptr && std::strstr(path, "C") != nullptr && std::strstr(path, "G") != nullptr);
  GetPath(root, path, sizeof(path));
  assert(std::strstr(path, "R") != nullptr);
}

static void test_get_id() {
  WorldRef w = GetCurrentWorld();
  NodeId root = CreateNode(w);
  HierarchyIterator it = Traverse(w, root);
  assert(GetId(it) == root);
  assert(it.GetId() == root);
}

static void test_set_active_get_active() {
  WorldRef w = GetCurrentWorld();
  NodeId n = CreateNode(w);
  assert(GetActive(n) == true);
  SetActive(n, false);
  assert(GetActive(n) == false);
  SetActive(n, true);
  assert(GetActive(n) == true);
}

static void test_iterator_single_use() {
  WorldRef w = GetCurrentWorld();
  NodeId root = CreateNode(w);
  HierarchyIterator it = Traverse(w, root);
  assert(it.IsValid());
  HierarchyIterator it2 = std::move(it);
  assert(!it.IsValid());
  assert(it2.IsValid());
  while (it2.IsValid()) it2.Next();
  assert(!it2.IsValid());
}

int main() {
  test_traverse_order();
  test_find_by_name();
  test_find_by_type();
  test_get_path();
  test_get_id();
  test_set_active_get_active();
  test_iterator_single_use();
  std::printf("te_scene_hierarchy_test passed\n");
  return 0;
}
