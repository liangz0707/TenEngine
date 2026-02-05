# 004-Scene 模块 ABI

- **契约**：[004-scene-public-api.md](./004-scene-public-api.md)（能力与类型描述）
- **本文件**：004-Scene 对外 ABI 显式表。
- **命名空间约定**：本分支统一使用 **`te::`** 作为全局命名空间；004-Scene 对外符号位于 **`te::scene`**，头文件路径为 **`te/scene/`**（与 `te::rhi`、`te::rendercore`、`te::pipelinecore` 等一致）。
- **参考**：Unity SceneManager（LoadScene/UnloadScene/GetActiveScene）、UE UWorld/Level 流式加载；场景图与层级对应 Unity Transform 层次、UE Actor 层级。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 类型与枚举

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | LoadSceneMode | 全局枚举 | 场景加载模式 | te/scene/SceneManager.h | LoadSceneMode::Single, LoadSceneMode::Additive | `enum class LoadSceneMode { Single, Additive };` Single=替换当前场景；Additive=叠加加载（参考 Unity） |
| 004-Scene | te::scene | SceneRef | struct/句柄 | 场景引用（关卡/世界容器） | te/scene/SceneTypes.h | SceneRef | 值类型或句柄；标识已加载场景，用于 SetActiveScene/UnloadScene |
| 004-Scene | te::scene | NodeId | struct/句柄 | 场景图节点 ID | te/scene/SceneTypes.h | NodeId | 值类型；标识场景图节点，层级路径与查找 |
| 004-Scene | te::scene | WorldRef | struct/句柄 | 世界容器引用（与 SceneRef 可统一） | te/scene/SceneTypes.h | WorldRef | 与 SceneRef 语义一致时可为同一类型别名 |

### 场景加载与活动场景（SceneManager 风格，对齐 Unity/UE）

**说明**：关卡资源的**唯一加载入口**为 013-Resource（Load(levelGuid)）。下列 LoadScene/LoadSceneAsync 为**高层便捷 API**，实现上应委托 013 Load(levelGuid)，013 再回调 004 CreateSceneFromDesc；004 不直接读 .level 文件。

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | — | 自由函数/静态 | 同步加载场景 | te/scene/SceneManager.h | LoadScene | `ISceneWorld* LoadScene(char const* pathOrName, LoadSceneMode mode);` 或 `LoadScene(ResourceId levelGuid, ...)`；内部委托 013 Load(levelGuid)，失败 nullptr；Single 时替换当前活动场景 |
| 004-Scene | te::scene | — | 自由函数/静态 | 异步加载场景 | te/scene/SceneManager.h | LoadSceneAsync | `void LoadSceneAsync(...)` 或返回 IAsyncLoadHandle；内部委托 013 RequestLoadAsync(levelGuid)；完成回调传入 ISceneWorld* |
| 004-Scene | te::scene | — | 自由函数/静态 | 卸载场景 | te/scene/SceneManager.h | UnloadScene | `void UnloadScene(SceneRef scene);` 销毁该场景下所有 Entity 与节点，释放引用；与 013-Resource 协同 |
| 004-Scene | te::scene | — | 自由函数/静态 | 获取当前活动场景 | te/scene/SceneManager.h | GetActiveScene | `SceneRef GetActiveScene();` 或 `ISceneWorld* GetActiveSceneWorld();` 当前活动场景/世界（参考 Unity GetActiveScene） |
| 004-Scene | te::scene | — | 自由函数/静态 | 设置当前活动场景 | te/scene/SceneManager.h | SetActiveScene | `void SetActiveScene(SceneRef scene);` 或 `void SetActiveSceneWorld(ISceneWorld* world);` 渲染/物理/脚本以该场景为根 |
| 004-Scene | te::scene | — | 自由函数/静态 | 已加载场景数量 | te/scene/SceneManager.h | GetLoadedSceneCount | `int GetLoadedSceneCount();` 当前已加载场景数（参考 Unity loadedSceneCount） |
| 004-Scene | te::scene | — | 自由函数/静态 | 按索引获取已加载场景 | te/scene/SceneManager.h | GetSceneAt | `SceneRef GetSceneAt(int index);` 或 `ISceneWorld* GetSceneWorldAt(int index);` 0 ≤ index < GetLoadedSceneCount() |

### 场景世界（ISceneWorld，Entity 容器，对齐 UE UWorld）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 场景内创建 Entity | te/scene/SceneWorld.h | ISceneWorld::CreateEntity | `IEntity* CreateEntity();` 在本世界内创建实体；调用方不拥有返回指针，由世界管理生命周期 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 销毁指定 Entity | te/scene/SceneWorld.h | ISceneWorld::DestroyEntity | `void DestroyEntity(IEntity* entity);` 须为本世界创建的 Entity |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 获取场景内 Entity 集合 | te/scene/SceneWorld.h | ISceneWorld::GetEntities | `IRange<IEntity*> GetEntities() const;` 或等价迭代器/列表；供渲染/脚本遍历 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 获取场景根节点 | te/scene/SceneWorld.h | ISceneWorld::GetRootNodes | `IRange<ISceneNode*> GetRootNodes() const;` 场景图根节点，无父节点 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 获取本世界引用 | te/scene/SceneWorld.h | ISceneWorld::GetSceneRef | `SceneRef GetSceneRef() const;` 或 `WorldRef GetWorldRef() const;` |
| 004-Scene | te::scene | — | 自由函数/单例 | 获取当前场景世界 | te/scene/SceneWorld.h | GetSceneWorld | `ISceneWorld* GetSceneWorld();` 等价于 GetActiveSceneWorld；可由 Subsystems 注册；调用方不拥有指针 |

### 场景图与节点（ISceneNode，对齐 Unity Transform / UE 层级）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取父节点 | te/scene/SceneNode.h | ISceneNode::GetParent | `ISceneNode* GetParent() const;` 根节点返回 nullptr |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置父节点 | te/scene/SceneNode.h | ISceneNode::SetParent | `void SetParent(ISceneNode* parent);` 父子关系变更后标记脏 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取子节点列表 | te/scene/SceneNode.h | ISceneNode::GetChildren | `IRange<ISceneNode*> GetChildren() const;` 或等价迭代器 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取局部变换 | te/scene/SceneNode.h | ISceneNode::GetLocalTransform | `Transform const& GetLocalTransform() const;` 或 Position/Rotation/Scale 分开；类型与 Core.Math 或本模块约定一致 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置局部变换 | te/scene/SceneNode.h | ISceneNode::SetLocalTransform | `void SetLocalTransform(Transform const& t);` 设置后标记脏 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取世界变换 | te/scene/SceneNode.h | ISceneNode::GetWorldTransform | `Transform const& GetWorldTransform() const;` 依赖 UpdateTransforms 已执行；只读 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取世界矩阵 | te/scene/SceneNode.h | ISceneNode::GetWorldMatrix | `float const* GetWorldMatrix() const;` 或 `Matrix4 const& GetWorldMatrix() const;` 4×4 行/列主序与 Core 约定一致 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取节点 ID | te/scene/SceneNode.h | ISceneNode::GetNodeId | `NodeId GetNodeId() const;` |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 获取层级路径 | te/scene/SceneNode.h | ISceneNode::GetPath | `void GetPath(char* outPath, size_t maxLen) const;` 或 `String GetPath() const;` 根到本节点的路径字符串 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 设置激活状态 | te/scene/SceneNode.h | ISceneNode::SetActive | `void SetActive(bool active);` 节点及子树是否参与更新/渲染 |
| 004-Scene | te::scene | ISceneNode | 抽象接口 | 查询是否激活 | te/scene/SceneNode.h | ISceneNode::IsActive | `bool IsActive() const;` 考虑父链：父未激活则本节点视为未激活 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 按拓扑更新世界变换 | te/scene/SceneWorld.h | ISceneWorld::UpdateTransforms | `void UpdateTransforms();` 遍历脏节点，父世界×局部=世界，清除脏标记；渲染/物理前调用 |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 按名称查找节点 | te/scene/SceneWorld.h | ISceneWorld::FindNodeByName | `ISceneNode* FindNodeByName(char const* name) const;` 层级中首次匹配；未找到 nullptr |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 按 ID 查找节点 | te/scene/SceneWorld.h | ISceneWorld::FindNodeById | `ISceneNode* FindNodeById(NodeId id) const;` 未找到 nullptr |
| 004-Scene | te::scene | ISceneWorld | 抽象接口 | 创建子节点 | te/scene/SceneWorld.h | ISceneWorld::CreateNode | `ISceneNode* CreateNode(ISceneNode* parent, char const* name);` parent 可为 nullptr 表示根下；名称可空 |

*来源：用户故事 US-scene-001（场景加载与切换）、US-scene-002（场景图与节点）；参考 Unity SceneManager、Transform 层级；UE UWorld/Level 流式与 Actor 层级。*

---

数据与接口 TODO 已迁移至本模块契约 [004-scene-public-api.md](./004-scene-public-api.md) 的 TODO 列表；本文件仅保留 ABI 表与实现说明。关卡加载入口现为 029-World，029 调 004 CreateSceneFromDesc(SceneDesc)。
