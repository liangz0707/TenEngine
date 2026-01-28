# TenEngine 模块文档索引

每个功能模块对应一个子文档，模块名称不以数字开头。文档结构统一为：

1. **模块详细功能**：功能描述、覆盖 UE/Unity 的能力
2. **子模块**：是否有子模块；若有则包含子模块说明、具体功能、子模块依赖图
3. **模块上下游**：直接上游依赖、下游消费者
4. **可能依赖的技术、工具**：第三方库、工具链、平台 API

---

## L0 基础层

| 模块 | 文档 | 说明 |
|------|------|------|
| Core | [core.md](core.md) | 内存、线程、平台、日志、数学、容器、模块加载 |
| Object | [object.md](object.md) | 反射、序列化、属性系统、类型注册 |
| Application | [application.md](application.md) | 应用生命周期、窗口、消息循环、主循环 |

## L1 场景/实体/输入/图形抽象

| 模块 | 文档 | 说明 |
|------|------|------|
| Scene | [scene.md](scene.md) | 场景图、层级、World/Level、激活/禁用 |
| Entity | [entity.md](entity.md) | 实体/组件模型或 ECS |
| Input | [input.md](input.md) | 输入抽象、键鼠/手柄/触摸 |
| Subsystems | [subsystems.md](subsystems.md) | 可插拔子系统描述符、注册、生命周期 |
| RHI | [rhi.md](rhi.md) | 图形 API 抽象（Vulkan/D3D12/Metal） |

## L2 渲染类型/内容/物理/动画/音频/UI

| 模块 | 文档 | 说明 |
|------|------|------|
| RenderCore | [render-core.md](render-core.md) | Shader 参数结构、Pass 协议、Uniform Buffer |
| Shader | [shader.md](shader.md) | Shader 编译、变体、预编译、Shader Graph |
| Material | [material.md](material.md) | 材质定义、参数、与 Shader 绑定、材质实例 |
| Mesh | [mesh.md](mesh.md) | 网格数据、LOD、蒙皮、顶点/索引 |
| Resource | [resource.md](resource.md) | 资源导入、同步/异步加载、流式、可寻址 |
| Physics | [physics.md](physics.md) | 碰撞、刚体、查询、2D/3D |
| Animation | [animation.md](animation.md) | 动画剪辑、骨骼动画、Timeline、状态机 |
| Audio | [audio.md](audio.md) | 音源、监听、混音、空间音效 |
| UICore | [ui-core.md](ui-core.md) | UI 布局、绘制、输入路由 |
| UI | [ui.md](ui.md) | 控件、画布、事件 |

## L3 管线与特性

| 模块 | 文档 | 说明 |
|------|------|------|
| PipelineCore | [pipeline-core.md](pipeline-core.md) | 命令缓冲格式、Pass 图协议、与 RHI 提交约定 |
| Pipeline | [pipeline.md](pipeline.md) | 场景收集、剔除、DrawCall、命令缓冲生成与提交 |
| Effects | [effects.md](effects.md) | 后处理、粒子/VFX、光照后处理 |
| 2D | [two-d.md](two-d.md) | 精灵、Tilemap、2D 物理、2D 渲染 |
| Terrain | [terrain.md](terrain.md) | 地形数据、LOD、绘制/刷 |

## L4 工具与扩展

| 模块 | 文档 | 说明 |
|------|------|------|
| Editor | [editor.md](editor.md) | 视口、场景树、属性面板、资源编辑、菜单 |
| Tools | [tools.md](tools.md) | 构建、批处理、CLI、插件/包管理 |
| Networking | [networking.md](networking.md) | 复制、RPC、客户端/服务端 |
| XR | [xr.md](xr.md) | AR/VR 子系统、头显、手柄 |

---

依赖关系总图见 [../dependency-graph-full.md](../dependency-graph-full.md)，全功能规格见 [../tenengine-full-module-spec.md](../tenengine-full-module-spec.md)。契约见 `specs/_contracts/`、[../contracts-and-specs-T0.md](../contracts-and-specs-T0.md)，命名 `NNN-modulename-public-api`。
