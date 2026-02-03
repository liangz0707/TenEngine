# TenEngine 模块详细规格目录

本目录按**依赖顺序 + 优先级 + 模块名**组织，每个模块一个子文档，文件名格式：`NNN-模块名.md`（如 `001-core.md`）。  
文档结构统一为：模块简要说明、详细功能描述、实现难度、操作的资源类型、子模块（若有）、模块上下游（交互数据类型与依赖图）、依赖的外部内容。

---

## 文档索引（按层级）

### L0 基础层

| 编号 | 文档 | 模块名 |
|------|------|--------|
| 001 | [001-core.md](001-core.md) | Core |
| 002 | [002-object.md](002-object.md) | Object |
| 003 | [003-application.md](003-application.md) | Application |

### L1 场景/实体/输入/图形抽象

| 编号 | 文档 | 模块名 |
|------|------|--------|
| 004 | [004-scene.md](004-scene.md) | Scene |
| 005 | [005-entity.md](005-entity.md) | Entity |
| 006 | [006-input.md](006-input.md) | Input |
| 007 | [007-subsystems.md](007-subsystems.md) | Subsystems |
| 008 | [008-rhi.md](008-rhi.md) | RHI |

### L2 渲染类型/内容/物理/动画/音频/UI

| 编号 | 文档 | 模块名 |
|------|------|--------|
| 009 | [009-render-core.md](009-render-core.md) | RenderCore |
| 010 | [010-shader.md](010-shader.md) | Shader |
| 011 | [011-material.md](011-material.md) | Material |
| 012 | [012-mesh.md](012-mesh.md) | Mesh |
| 013 | [013-resource.md](013-resource.md) | Resource |
| 014 | [014-physics.md](014-physics.md) | Physics |
| 015 | [015-animation.md](015-animation.md) | Animation |
| 016 | [016-audio.md](016-audio.md) | Audio |
| 017 | [017-ui-core.md](017-ui-core.md) | UICore |
| 018 | [018-ui.md](018-ui.md) | UI |

### L3 管线与特性

| 编号 | 文档 | 模块名 |
|------|------|--------|
| 019 | [019-pipeline-core.md](019-pipeline-core.md) | PipelineCore |
| 020 | [020-pipeline.md](020-pipeline.md) | Pipeline |
| 021 | [021-effects.md](021-effects.md) | Effects |
| 022 | [022-2d.md](022-2d.md) | 2D |
| 023 | [023-terrain.md](023-terrain.md) | Terrain |

### L4 工具与扩展

| 编号 | 文档 | 模块名 |
|------|------|--------|
| 024 | [024-editor.md](024-editor.md) | Editor |
| 025 | [025-tools.md](025-tools.md) | Tools |
| 026 | [026-networking.md](026-networking.md) | Networking |
| 027 | [027-xr.md](027-xr.md) | XR |

---

依赖关系总图见 [../dependency-graph-full.md](../dependency-graph-full.md)，全功能规格见 [../tenengine-full-module-spec.md](../tenengine-full-module-spec.md)。
