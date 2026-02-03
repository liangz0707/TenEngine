# lifecycle 领域 · 用户故事索引

| 编号 | 标题 | 涉及模块 | 文档 |
|------|------|----------|------|
| US-lifecycle-001 | 应用启动并进入主循环 | 001-Core, 003-Application, 007-Subsystems | [US-lifecycle-001-app-startup-main-loop.md](./US-lifecycle-001-app-startup-main-loop.md) |
| US-lifecycle-002 | 用户可选择编辑器启动或游戏模式启动 | 001-Core, 003-Application, 007-Subsystems, 024-Editor | [US-lifecycle-002-editor-or-game-mode-startup.md](./US-lifecycle-002-editor-or-game-mode-startup.md) |
| US-lifecycle-003 | 所有内容分配与现成分配均有统一接口 | 001-Core | [US-lifecycle-003-unified-allocation-interface.md](./US-lifecycle-003-unified-allocation-interface.md) |
| US-lifecycle-004 | 统一数学库（矩阵/四元数/空间变换全局调用） | 001-Core | [US-lifecycle-004-unified-math-library.md](./US-lifecycle-004-unified-math-library.md) |
| US-lifecycle-005 | 统一 Check 宏、编译选项控制 Check、容易错误处不用异常；渲染支持 Debug/Hybrid/Resource | 001-Core, 020-Pipeline | [US-lifecycle-005-check-macros-render-modes-no-exception.md](./US-lifecycle-005-check-macros-render-modes-no-exception.md) |
| US-lifecycle-006 | 引擎支持 Android/iOS 等平台、Vulkan/Metal/GLSL/DXIL 等接口、通过宏选择代码路径 | 001-Core, 008-RHI, 010-Shader | [US-lifecycle-006-platform-rhi-macros.md](./US-lifecycle-006-platform-rhi-macros.md) |
| US-lifecycle-007 | 应用退出与清理（逆序关闭子系统、释放资源、日志冲刷） | 007-Subsystems, 003-Application, 001-Core | [US-lifecycle-007-app-shutdown-and-cleanup.md](./US-lifecycle-007-app-shutdown-and-cleanup.md) |

*与顶层索引的关系：`specs/user-stories/000-user-stories-index.md` 仅做领域导航；各领域详情见本表。*
