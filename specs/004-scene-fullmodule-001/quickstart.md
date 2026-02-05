# Quickstart: 004-Scene Full Module

**Branch**: 004-scene-fullmodule-001

## 1. 构建

```bash
# 从仓库根（或 004-scene worktree 根）配置并构建
cmake -B build -S .
cmake --build build
```

依赖 001-core、002-object 通过 TenEngineHelpers 以源码引入；见 `CMakeLists.txt` 与 `docs/engine-build-module-convention.md`。

## 2. 最小验证（契约 API）

```cpp
#include "scene/world.hpp"
#include "scene/scene_graph.hpp"
#include "scene/hierarchy.hpp"

using namespace te::scene;

// 获取当前 World，创建节点，设置父子与变换，更新世界变换
WorldRef w = GetCurrentWorld();
NodeId root = CreateNode(w);
NodeId child = CreateNode(w);
SetParent(child, root);
SetLocalTransform(child, someTransform);
SetDirty(child);
UpdateTransforms(w);
Transform worldT = GetWorldTransform(child);

// 层级遍历与查找
HierarchyIterator it = Traverse(w, root);
while (it.IsValid()) { NodeId id = GetId(it); it.Next(); }
```

## 3. 测试

```bash
ctest --test-dir build
# 或直接运行
./build/Debug/te_scene_test.exe
./build/Debug/te_scene_hierarchy_test.exe
```

## 4. 规约与契约

- **规约**：`docs/module-specs/004-scene.md`
- **契约**：`specs/_contracts/004-scene-public-api.md`
- **全量 ABI（实现参考）**：`specs/004-scene-fullmodule-001/contracts/004-scene-ABI-full.md`
