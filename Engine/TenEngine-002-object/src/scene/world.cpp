// 004-Scene: World implementation. Single default world; GetCurrentWorld; AddWorld; SetActiveWorld (pending next UpdateTransforms).

#include "scene/world.hpp"
#include "scene/scene_graph.hpp"
#include <vector>
#include <cstdlib>

namespace te {
namespace scene {

struct WorldImpl {
  void* user = nullptr;  // reserved for root entity / level handle
  WorldImpl() = default;
};
static std::vector<WorldImpl> g_worlds;
static WorldImpl* g_current = nullptr;
static WorldImpl* g_pending = nullptr;  // applied at next UpdateTransforms

WorldRef GetCurrentWorld() {
  if (g_current) return g_current;
  g_worlds.emplace_back();
  g_current = &g_worlds.back();
  return g_current;
}

void SetActiveWorld(WorldRef world) {
  g_pending = static_cast<WorldImpl*>(world);
}

WorldRef AddWorld() {
  g_worlds.emplace_back();
  return &g_worlds.back();
}

void ApplyPendingActiveWorld() {
  if (g_pending) {
    g_current = g_pending;
    g_pending = nullptr;
  }
}

}  // namespace scene
}  // namespace te
