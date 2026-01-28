# T0 模块依赖图（27 模块）

用于多 Agent 协作时快速查看：**谁依赖谁**、**改某个模块会影响谁**。接口边界以 `specs/_contracts/` 下契约为准。  
**完整依赖图（Mermaid、矩阵、边列表）** 见 **`docs/dependency-graph-full.md`**。

## 层级与模块（T0 最新架构）

- **L0 基础**：001-Core, 002-Object, 003-Application  
- **L1**：004-Scene, 005-Entity, 006-Input, 007-Subsystems, 008-RHI  
- **L2**：009-RenderCore, 010-Shader, 011-Material, 012-Mesh, 013-Resource, 014-Physics, 015-Animation, 016-Audio, 017-UICore, 018-UI  
- **L3**：019-PipelineCore, 020-Pipeline, 021-Effects, 022-2D, 023-Terrain  
- **L4**：024-Editor, 025-Tools, 026-Networking, 027-XR  

## 依赖表（下游 → 上游，直接依赖）

**每模块一契约**：每行「本模块契约」即该模块对外提供的契约文件名；「契约文件」为该模块所依赖的上游契约。

| 模块 | 直接依赖 | 契约文件（依赖） | 本模块契约 |
|------|----------|------------------|------------|
| 001-Core | — | 无（根） | 001-core-public-api |
| 002-Object | Core | 001-core-public-api | 002-object-public-api |
| 003-Application | Core | 001-core-public-api | 003-application-public-api |
| 004-Scene | Core, Object | 001-core-public-api, 002-object-public-api | 004-scene-public-api |
| 005-Entity | Core, Object, Scene | 001-core-public-api, 002-object-public-api, 004-scene-public-api | 005-entity-public-api |
| 006-Input | Core, Application | 001-core-public-api, 003-application-public-api | 006-input-public-api |
| 007-Subsystems | Core, Object | 001-core-public-api, 002-object-public-api | 007-subsystems-public-api |
| 008-RHI | Core | 001-core-public-api | 008-rhi-public-api |
| 009-RenderCore | Core, RHI | 001-core-public-api, 008-rhi-public-api | 009-rendercore-public-api |
| 010-Shader | Core, RHI, RenderCore | 001-core-public-api, 008-rhi-public-api, 009-rendercore-public-api | 010-shader-public-api |
| 011-Material | RenderCore, Shader | 009-rendercore-public-api, 010-shader-public-api | 011-material-public-api |
| 012-Mesh | Core, RenderCore | 001-core-public-api, 009-rendercore-public-api | 012-mesh-public-api |
| 013-Resource | Core, Object | 001-core-public-api, 002-object-public-api | 013-resource-public-api |
| 014-Physics | Core, Scene, Entity | 001-core-public-api, 004-scene-public-api, 005-entity-public-api | 014-physics-public-api |
| 015-Animation | Core, Object, Entity | 001-core-public-api, 002-object-public-api, 005-entity-public-api | 015-animation-public-api |
| 016-Audio | Core, Resource | 001-core-public-api, 013-resource-public-api | 016-audio-public-api |
| 017-UICore | Core, Application, Input | 001-core-public-api, 003-application-public-api, 006-input-public-api | 017-uicore-public-api |
| 018-UI | UICore | 017-uicore-public-api | 018-ui-public-api |
| 019-PipelineCore | RHI, RenderCore | 008-rhi-public-api, 009-rendercore-public-api | 019-pipelinecore-public-api |
| 020-Pipeline | Core, Scene, Entity, PipelineCore, RenderCore, Shader, Material, Mesh, Resource | 001/004/005/019/009/010/011/012/013 等 | 020-pipeline-public-api |
| 021-Effects | PipelineCore, RenderCore, Shader | 019-pipelinecore-public-api, 009-rendercore-public-api, 010-shader-public-api | 021-effects-public-api |
| 022-2D | Core, Resource, Physics, Pipeline, RenderCore | 001, 013, 014, 020-pipeline-public-api, 009 等 | 022-2d-public-api |
| 023-Terrain | Core, Resource, Mesh, Pipeline, RenderCore | 001, 013, 012, 020-pipeline-public-api, 009 等 | 023-terrain-public-api |
| 024-Editor | Core, Application, Input, RHI, Resource, Scene, Entity, Pipeline, UI | 多个 L0–L3 契约 | 024-editor-public-api |
| 025-Tools | 按需 | 按实际依赖 | 025-tools-public-api |
| 026-Networking | Core, Entity | 001-core-public-api, 005-entity-public-api | 026-networking-public-api |
| 027-XR | Core, Subsystems, Input, Pipeline | 001, 007, 006, 020-pipeline-public-api | 027-xr-public-api |

## 谁被谁依赖（上游 → 下游，修改接口时需通知）

| 提供方 | 依赖它的下游（节选） |
|--------|----------------------|
| 001-Core | 几乎所有模块 |
| 002-Object | Scene, Entity, Subsystems, Resource, Animation 等 |
| 003-Application | Input, UICore, Pipeline, Editor 等 |
| 004-Scene | Entity, Physics, Pipeline, Editor 等 |
| 005-Entity | Physics, Animation, Pipeline, Editor, Networking 等 |
| 006-Input | UICore, Editor, XR 等 |
| 007-Subsystems | XR 等 |
| 008-RHI | RenderCore, Shader, PipelineCore, Pipeline, Editor, XR |
| 009-RenderCore | Shader, Material, Mesh, PipelineCore, Pipeline, Effects, 2D, Terrain |
| 010-Shader | Material, Pipeline, Effects |
| 011-Material | Pipeline |
| 012-Mesh | Pipeline, Terrain, Animation |
| 013-Resource | Audio, Pipeline, 2D, Terrain, Editor |
| 014-Physics | 2D |
| 015-Animation | Pipeline, Mesh, Editor |
| 017-UICore | UI, Editor |
| 018-UI | Editor |
| 019-PipelineCore | Pipeline, Effects |
| 020-Pipeline | Effects, 2D, Terrain, Editor, XR |
| 021-Effects | Pipeline, Editor |
| 022-2D | Editor |
| 023-Terrain | Editor |

**边界契约**：020-Pipeline ↔ 008-RHI 的命令缓冲与提交约定见 `pipeline-to-rci.md`，020、008 实现须遵循。

**流程**：修改某模块的公开 API → 在 **T0-contracts** 分支更新对应契约 → 在 `000-module-dependency-map.md` 中确认下游 → 通知或创建下游适配任务。

---

*本文件与 `docs/dependency-graph-full.md`、`docs/tenengine-full-module-spec.md` 保持一致；任一 Agent 修改依赖关系时请同步更新。*
