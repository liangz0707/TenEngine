# Research: 004-Scene Full Module

**Branch**: 004-scene-fullmodule-001 | **Phase**: 0

## Decisions

| Topic | Decision | Rationale |
|-------|----------|------------|
| Tech stack | C++17, CMake | Per spec and TenEngine convention; no NEEDS CLARIFICATION. |
| API style | Contract-first: public-api free functions + opaque handles (WorldRef, NodeId); ABI may extend with OO-style (ISceneWorld/ISceneNode) where already in ABI | Single source of truth: `specs/_contracts/004-scene-public-api.md`; ABI `004-scene-ABI.md` defines full implementable surface; Unity/UE used as reference for semantics. |
| Transform type | Use struct with position, rotation, scale (float[3], float[4], float[3]) or align with 001-Core.Math when available | Contract allows Core.Math or local type; current implementation uses local Transform in types.hpp. |
| HierarchyIterator | Single-use, move-only; Traverse/FindByName/FindByType return by value | Per contract; implementation already follows. |
| SetActiveWorld timing | Apply at next UpdateTransforms or frame boundary | Per contract and spec; avoids mid-frame world switch. |
| Level/LoadScene | LoadLevel(WorldRef, ...)/UnloadLevel(WorldRef, LevelHandle) per contract; ABI TODOs add LevelAssetDesc, SceneNodeDesc, GetNodeModelGuid, GetNodeEntityPrefabGuid, type registration | 013-Resource not yet fixed; implement stub or minimal signature; ABI TODOs are required additions for this feature. |

## Alternatives Considered

- **ISceneWorld/ISceneNode vs free functions only**: ABI file already defines both; public contract is free-function + handles. We keep ABI as full surface (implementations may expose free functions that delegate to internal world/node objects).
- **LoadScene vs LoadLevel**: Contract uses LoadLevel/UnloadLevel/LevelHandle; ABI uses LoadScene/UnloadScene/SceneRef. Plan keeps both naming in ABI for Unity alignment; implementation satisfies contract (LoadLevel ↔ LoadScene semantics).

## References

- Unity: SceneManager (LoadScene, GetActiveScene, SetActiveScene), Transform hierarchy.
- Unreal: UWorld, ULevel streaming, Actor hierarchy.
- Existing 004-scene implementation: `src/scene/` (world, scene_graph, hierarchy, level); contract-only API.
