# 029-World 模块描述

## 1. 模块简要说明

World 提供**场景资源管理**与 **Level 实际资源引用**：持有关卡资源句柄、与 013-Resource 协作完成 Level 加载、并**底层依赖 004-Scene** 进行场景管理。调用 004 前将 013 的 **LevelAssetDesc 转为 004 定义的 SceneDesc** 再调用 004 CreateSceneFromDesc(SceneDesc, …)，避免 004 依赖 029/013。004-Scene 仅负责逻辑场景图，不依赖 Resource。依赖 Scene、Resource。

## 2. 详细功能描述

- **Level 资源**：Level 作为 ILevelResource；013 Load(levelGuid) 后，由 World 参与创建 Level 实例（013 读 .level、反序列化 LevelAssetDesc、加载依赖后，通过 World 或注册的 Level 工厂完成创建）；World **将 LevelAssetDesc 转为 004 的 SceneDesc 与按节点的不透明句柄**后，调用 004 **CreateSceneFromDesc**(SceneDesc, …) 初始化场景图，并持有关卡句柄与 SceneRef。
- **实际资源引用**：World 持有 Level 与 013 的资源句柄/ResourceId、与场景的对应关系；下游（Pipeline、Editor 等）需“当前关卡”时可通过 World 获取 SceneRef 或 Level 句柄。
- **不替代 Scene**：场景图、层级遍历、World/Level 容器、激活/禁用等仍由 004-Scene 提供；World 仅负责“Level 资源侧”与对 004 的调用。

## 3. 实现难度

**中**。与 013 的 Level 加载流程、与 004 CreateSceneFromDesc 的对接需清晰；避免与 004 职责重叠。

## 4. 操作的资源类型

- **输入**：013 传入的 LevelAssetDesc、nodeModelRefs（或由 World 向 013 请求加载后获得）。
- **转换与输出**：World 将 LevelAssetDesc 转为 004 定义的 SceneDesc 与按节点的不透明句柄后调用 004；输出 Level 句柄、SceneRef（来自 004）、与 013 的 ResourceId/句柄对应关系。

## 5. 是否有子模块

可选（按需拆 Level 生命周期、流式等）。

## 6. 模块上下游

### 6.1 和上下游交互、传递的数据类型

- **上游**：004-Scene（CreateSceneFromDesc、UnloadScene、SceneRef）、013-Resource（Load、ResourceId、句柄解析）。
- **下游**：020-Pipeline、024-Editor（可选；需 Level 句柄或当前关卡 SceneRef 时）。向下游提供：LevelHandle、GetCurrentLevelScene、UnloadLevel 等。

### 6.2 上下游依赖图

- 依赖关系见 `specs/_contracts/000-module-dependency-map.md`。

## 7. 依赖的外部内容

| 类别 | 内容 |
|------|------|
| **004-Scene** | CreateSceneFromDesc、UnloadScene、SceneRef、层级遍历由 Scene 提供 |
| **013-Resource** | Load(levelGuid)、ResourceId、句柄；Level 为一种资源类型 |
| **协议** | 无 |
