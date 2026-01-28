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

| 模块 | 直接依赖 | 契约文件 |
|------|----------|----------|
| 001-Core | — | 无（根） |
| 002-Object | Core | core-public-api |
| 003-Application | Core | core-public-api |
| 004-Scene | Core, Object | core-public-api, object-public-api |
| 005-Entity | Core, Object, Scene | core-public-api, object-public-api, scene-public-api |
| 006-Input | Core, Application | core-public-api, application-public-api |
| 007-Subsystems | Core, Object | core-public-api, object-public-api |
| 008-RHI | Core | core-public-api |
| 009-RenderCore | Core, RHI | core-public-api, rhi-public-api |
| 010-Shader | Core, RHI, RenderCore | core-public-api, rhi-public-api, render-core-api |
| 011-Material | RenderCore, Shader | render-core-api, shader-public-api |
| 012-Mesh | Core, RenderCore | core-public-api, render-core-api |
| 013-Resource | Core, Object | core-public-api, object-public-api |
| 014-Physics | Core, Scene, Entity | core-public-api, scene-public-api, entity-public-api |
| 015-Animation | Core, Object, Entity | core-public-api, object-public-api, entity-public-api |
| 016-Audio | Core, Resource | core-public-api, resource-public-api |
| 017-UICore | Core, Application, Input | core-public-api, application-public-api, input-public-api |
| 018-UI | UICore | ui-core-api |
| 019-PipelineCore | RHI, RenderCore | rhi-public-api, render-core-api |
| 020-Pipeline | Core, Scene, Entity, PipelineCore, RenderCore, Shader, Material, Mesh, Resource | 多个 L0–L2 契约 |
| 021-Effects | PipelineCore, RenderCore, Shader | pipeline-core-api, render-core-api, shader-public-api |
| 022-2D | Core, Resource, Physics, Pipeline, RenderCore | 多个 |
| 023-Terrain | Core, Resource, Mesh, Pipeline, RenderCore | 多个 |
| 024-Editor | Core, Application, Input, RHI, Resource, Scene, Entity, Pipeline, UI | 多个 |
| 025-Tools | 按需 | 按实际依赖 |
| 026-Networking | Core, Entity | core-public-api, entity-public-api |
| 027-XR | Core, Subsystems, Input, Pipeline | 多个 |

## 谁被谁依赖（上游 → 下游，修改接口时需通知）

| 提供方 | 依赖它的下游（节选） |
|--------|----------------------|
| 001-Core | 几乎所有模块 |
| 002-Object | Scene, Entity, Subsystems, Resource, Animation 等 |
| 008-RHI | RenderCore, Shader, PipelineCore, Pipeline, Editor, XR |
| 009-RenderCore | Shader, Material, Mesh, PipelineCore, Pipeline, Effects, 2D, Terrain |
| 020-Pipeline | Effects, 2D, Terrain, Editor, XR |

**流程**：修改某模块的公开 API → 在 **T0-contracts** 分支更新对应契约 → 在 `000-module-dependency-map.md` 中确认下游 → 通知或创建下游适配任务。

---

*本文件与 `docs/dependency-graph-full.md`、`docs/tenengine-full-module-spec.md` 保持一致；任一 Agent 修改依赖关系时请同步更新。*
