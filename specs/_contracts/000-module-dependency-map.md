# 模块依赖图（Module Dependency Map）

用于多 Agent 协作时快速查看：**谁依赖谁**、**改某个模块会影响谁**。接口边界以 `specs/_contracts/` 下契约为准。

**依赖表与规则**、**依赖边列表**见 **`docs/engine-modules-and-architecture.md`** §二、§三。

---

## 依赖表（下游 → 上游）

| 模块 | 直接依赖 | 契约文件 |
|------|----------|----------|
| 001-Core | — | 无（根模块） |
| 002-Object | Core | 001-core-public-api.md |
| 003-Application | Core | 001-core-public-api.md |
| 004-Scene | Core | 001-core-public-api.md |
| 005-Entity | Core, Object, Scene | 001-core-public-api.md, 002-object-public-api.md, 004-scene-public-api.md |
| 006-Input | Core, Application | 001-core-public-api.md, 003-application-public-api.md |
| 007-Subsystems | Core, Object | 001-core-public-api.md, 002-object-public-api.md |
| 008-RHI | Core | 001-core-public-api.md |
| 009-RenderCore | Core, RHI | 001-core-public-api.md, 008-rhi-public-api.md |
| 010-Shader | Core, RHI, RenderCore, Resource, Object | 001-core-public-api.md, 008-rhi-public-api.md, 009-rendercore-public-api.md, 013-resource-public-api.md, 002-object-public-api.md |
| 011-Material | RenderCore, Shader, Texture, Resource | 009-rendercore-public-api.md, 010-shader-public-api.md, 028-texture-public-api.md, 013-resource-public-api.md |
| 012-Mesh | Core, RHI, RenderCore, Resource | 001-core-public-api.md, 008-rhi-public-api.md, 009-rendercore-public-api.md, 013-resource-public-api.md |
| 013-Resource | Core, Object, Texture | 001-core-public-api.md, 002-object-public-api.md, 028-texture-public-api.md |
| 014-Physics | Core, Scene, Entity | 001-core-public-api.md, 004-scene-public-api.md, 005-entity-public-api.md |
| 015-Animation | Core, Object, Entity | 001-core-public-api.md, 002-object-public-api.md, 005-entity-public-api.md |
| 016-Audio | Core, Resource | 001-core-public-api.md, 013-resource-public-api.md |
| 017-UICore | Core, Application, Input | 001-core-public-api.md, 003-application-public-api.md, 006-input-public-api.md |
| 018-UI | UICore | 017-uicore-public-api.md |
| 019-PipelineCore | RHI, RenderCore | 008-rhi-public-api.md, 009-rendercore-public-api.md |
| 020-Pipeline | Core, Scene, Entity, PipelineCore, RenderCore, Shader, Material, Mesh, Texture, Resource, Effects；Animation（可选） | 见 pipeline-to-rci.md 及各上游契约 |
| 021-Effects | PipelineCore, RenderCore, Shader, Texture | 019-pipelinecore-public-api.md, 009-rendercore-public-api.md, 010-shader-public-api.md, 028-texture-public-api.md |
| 022-2D | Core, Resource, Physics, Pipeline, RenderCore, Texture | 001-core-public-api.md, 013-resource-public-api.md, 014-physics-public-api.md, 020-pipeline-public-api.md, 009-rendercore-public-api.md, 028-texture-public-api.md |
| 023-Terrain | Core, Resource, Mesh, Pipeline, RenderCore, Texture | 001-core-public-api.md, 013-resource-public-api.md, 012-mesh-public-api.md, 020-pipeline-public-api.md, 009-rendercore-public-api.md, 028-texture-public-api.md |
| 024-Editor | Core, Application, Input, RHI, Resource, Scene, Entity, Pipeline, UI, Texture | 各对应 NNN-modulename-public-api.md（含 028-texture-public-api.md） |
| 025-Tools | 按需 | 025-tools-public-api.md（按需） |
| 026-Networking | Core, Entity | 001-core-public-api.md, 005-entity-public-api.md |
| 027-XR | Core, Subsystems, Input, Pipeline | 001-core-public-api.md, 007-subsystems-public-api.md, 006-input-public-api.md, 020-pipeline-public-api.md |
| 028-Texture | Core, Object, RHI, RenderCore, Resource, 030-DeviceResourceManager | 001-core-public-api.md, 002-object-public-api.md, 008-rhi-public-api.md, 009-rendercore-public-api.md, 013-resource-public-api.md, 030-device-resource-manager-public-api.md |
| 029-World | Scene, Resource, Entity | 004-scene-public-api.md, 013-resource-public-api.md, 005-entity-public-api.md |
| 030-DeviceResourceManager | Core, RHI, Resource | 001-core-public-api.md, 008-rhi-public-api.md, 013-resource-public-api.md |

---

## 谁被谁依赖（上游 → 下游）

修改某模块的**对外接口**时，需考虑以下**下游**模块的兼容性：

| 提供方模块 | 依赖它的下游 |
|------------|--------------|
| 001-Core | 002, 003, 004, 005, 006, 007, 008, 009, 010, 012, 013, 014, 015, 016, 017, 020, 022, 023, 024, 026, 027, 028 |
| 002-Object | 004, 005, 007, 010（ShaderAssetDesc 注册）, 013, 015 |
| 003-Application | 006, 017, 024 |
| 004-Scene | 005, 014, 020, 024, 029 |
| 005-Entity | 014, 015, 020, 024, 026 |
| 006-Input | 017, 024, 027 |
| 007-Subsystems | 027 |
| 008-RHI | 009, 010, 012（EnsureDeviceResources）, 019, 020, 024, 028 |
| 009-RenderCore | 010, 011, 012, 019, 020, 021, 022, 023, 028 |
| 010-Shader | 011, 020, 021 |
| 011-Material | 013（CreateMaterial）, 020, 028（贴图引用） |
| 012-Mesh | 013（CreateMesh）, 020, 023, 015 |
| 013-Resource | 010（Load/Shader）, 011（CreateMaterial）, 012（CreateMesh）, 016, 020, 022, 023, 024, 028（CreateTexture）, 029（Level 加载时 World 使用 013） |
| 014-Physics | 022 |
| 015-Animation | 020（可选）, 012, 024 |
| 016-Audio | — |
| 017-UICore | 018, 024 |
| 018-UI | 024 |
| 019-PipelineCore | 020, 021 |
| 020-Pipeline | 021, 022, 023, 024, 027；与 008 边界见 pipeline-to-rci.md |
| 021-Effects | 020, 024 |
| 022-2D | 024 |
| 023-Terrain | 024 |
| 024-Editor | — |
| 025-Tools | — |
| 026-Networking | — |
| 027-XR | — |
| 028-Texture | 011, 013（CreateTexture）, 020, 021, 022, 023, 024 |
| 029-World | 020, 024（可选；需 Level 句柄时） |
| 030-DeviceResourceManager | 028（EnsureDeviceResources 创建 GPU 纹理） |

**流程**：修改某模块的公开 API → 更新对应 `NNN-modulename-public-api.md` 与 ABI → 在上表中查「依赖它的下游」，确认下游 spec 或实现是否需要同步修改。

---

*本表「直接依赖」与 `docs/engine-modules-and-architecture.md` §二、§三 一致；修改依赖时请同步更新该文档与 `docs/module-specs/` 及各 spec 的 Dependencies。*
