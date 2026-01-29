// 004-Scene: Level stubs. LoadLevel, UnloadLevel; integrate with 013-Resource when agreed.

#include "scene/level.hpp"

namespace ten {
namespace scene {

LevelHandle LoadLevel(WorldRef world, void* resourceHandleOrPath) {
  (void)world; (void)resourceHandleOrPath;
  return nullptr;
}

void UnloadLevel(WorldRef world, LevelHandle level) {
  (void)world; (void)level;
}

}  // namespace scene
}  // namespace ten
