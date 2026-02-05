// 004-Scene: Unit tests for scene graph (CreateNode, SetParent, GetParent, GetChildren, Local/WorldTransform, SetDirty, UpdateTransforms, SetParent cycle rejection).

#include "scene/scene_graph.hpp"
#include "scene/world.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>

using namespace te::scene;

static void test_create_and_parent() {
  WorldRef w = GetCurrentWorld();
  assert(w != nullptr);
  NodeId root = CreateNode(w);
  assert(root != nullptr);
  NodeId c1 = CreateNode(w);
  assert(c1 != nullptr);
  bool ok = SetParent(c1, root);
  assert(ok);
  assert(GetParent(c1) == root);
  assert(GetParent(root) == nullptr);
  NodeId buf[4];
  size_t cnt = 4;
  GetChildren(root, buf, &cnt);
  assert(cnt == 1);
  assert(buf[0] == c1);
}

static void test_local_world_transform() {
  WorldRef w = GetCurrentWorld();
  NodeId a = CreateNode(w);
  NodeId b = CreateNode(w);
  SetParent(b, a);
  Transform t;
  t.position[0] = 1.f; t.position[1] = 2.f; t.position[2] = 3.f;
  SetLocalTransform(b, t);
  SetDirty(b);
  UpdateTransforms(w);
  Transform wb = GetWorldTransform(b);
  assert(wb.position[0] == 1.f && wb.position[1] == 2.f && wb.position[2] == 3.f);
}

static void test_cycle_rejection() {
  WorldRef w2 = AddWorld();
  SetActiveWorld(w2);
  UpdateTransforms(w2);
  WorldRef w = GetCurrentWorld();
  assert(w == w2);
  NodeId n1 = CreateNode(w);
  NodeId n2 = CreateNode(w);
  NodeId n3 = CreateNode(w);
  SetParent(n2, n1);
  SetParent(n3, n2);
  bool ok = SetParent(n1, n3);
  assert(!ok);
  assert(GetParent(n1) == nullptr);
}

int main() {
  test_create_and_parent();
  test_local_world_transform();
  test_cycle_rejection();
  std::printf("te_scene_test passed\n");
  return 0;
}
