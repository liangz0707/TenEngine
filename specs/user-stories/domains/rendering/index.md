# rendering 领域 · 用户故事索引

| 编号 | 标题 | 涉及模块 | 文档 |
|------|------|----------|------|
| US-rendering-001 | 一帧渲染（从场景/相机到屏幕） | 004-Scene, 005-Entity, 019-PipelineCore, 020-Pipeline, 008-RHI | [US-rendering-001-one-frame-render.md](./US-rendering-001-one-frame-render.md) |
| US-rendering-002 | 流水线式多帧渲染（2–4 帧在途） | 019-PipelineCore, 020-Pipeline, 008-RHI | [US-rendering-002-pipelined-multi-frame-render.md](./US-rendering-002-pipelined-multi-frame-render.md) |
| US-rendering-003 | 渲染通过 FrameGraph 组织，AddPass 配置 Pass（场景、剔除、物体类型、输出、收集物体） | 019-PipelineCore, 020-Pipeline, 004-Scene, 005-Entity, 008-RHI | [US-rendering-003-framegraph-add-pass.md](./US-rendering-003-framegraph-add-pass.md) |
| US-rendering-004 | 多线程执行：主循环/游戏更新(A)/构建管线(B)/多线程收集 RenderItem(C)/逻辑 CB(D)/Device 线程提交(E) | 003-Application, 019-PipelineCore, 020-Pipeline, 008-RHI | [US-rendering-004-multithreaded-pipeline-stages.md](./US-rendering-004-multithreaded-pipeline-stages.md) |
| US-rendering-005 | Shader 支持 HLSL/GLSL、宏切换代码路径、游戏中动态切换宏、实时更新（热重载） | 010-Shader, 008-RHI, 011-Material, 020-Pipeline, 021-Effects | [US-rendering-005-shader-formats-macros-hot-reload.md](./US-rendering-005-shader-formats-macros-hot-reload.md) |
| US-rendering-006 | 模型渲染 = Mesh + Material 组织；Model 引用 Mesh/Material；Material 保存 Shader/贴图/参数；Mesh 来自 OBJ/FBX 等 | 013-Resource, 012-Mesh, 011-Material, 010-Shader, 020-Pipeline | [US-rendering-006-model-mesh-material-organization.md](./US-rendering-006-model-mesh-material-organization.md) |
| US-rendering-007 | 渲染资源显式控制位置：CreateRenderItem、CollectCommandBuffer、SubmitCommandBuffer、PrepareRenderMaterial/Mesh、CreateDeviceResource/UpdateDeviceResource | 019-PipelineCore, 020-Pipeline, 008-RHI | [US-rendering-007-render-resource-control-points.md](./US-rendering-007-render-resource-control-points.md) |

*与顶层索引的关系：`specs/user-stories/000-user-stories-index.md` 仅做领域导航；各领域详情见本表。*
