# US-scene-001：场景加载与切换

- **标题**：应用可**加载场景**（同步或异步）、**切换当前活动场景**；场景作为 Entity 容器，加载时创建根节点与实体，卸载时释放；与 Resource 对接场景资源，与 Entity 对接生命周期。
- **编号**：US-scene-001

---

## 1. 角色/触发

- **角色**：游戏逻辑、Editor
- **触发**：需要**加载**某场景（如关卡、UI 场景）、**切换**当前活动场景（单场景模式或多场景叠加）；加载可同步或异步，与 Resource 统一加载接口一致。

---

## 2. 端到端流程

1. 调用方向 **Scene** 模块请求 **loadScene(path 或 ResourceId)** 或 **loadSceneAsync(path, callback)**；Scene 通过 **Resource** 加载场景资源（或直接读场景文件），解析为场景图与 Entity 列表。
2. **Scene** 创建 **ISceneWorld**（或等价场景容器）、根节点、并在其下创建 **Entity**（通过 005-Entity）；Entity 与 Component 数据由场景资源反序列化填充（可选依赖 002-Object 反射）。
3. **切换活动场景**：设置当前活动场景为刚加载的场景；渲染/物理/脚本等系统以当前活动场景为根进行更新与绘制；可单场景（替换）或多场景叠加（Additive）。
4. **卸载场景**：调用 **unloadScene**；Scene 销毁该场景下所有 Entity 与节点、释放引用；Resource 按引用计数或策略卸载场景资源。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 004-Scene | loadScene、loadSceneAsync、unloadScene、getActiveScene、setActiveScene；ISceneWorld、场景容器与 Entity 生命周期 |
| 013-Resource | 场景资源统一 requestLoadAsync、场景资源格式与依赖 |
| 005-Entity | createEntity、destroyEntity、随场景加载/卸载创建/销毁 Entity |

---

## 4. 每模块职责与 I/O

### 004-Scene

- **职责**：提供 **loadScene**、**loadSceneAsync**、**unloadScene**；**getActiveScene**、**setActiveScene**；ISceneWorld 作为 Entity 容器；场景加载时创建根节点与 Entity，卸载时销毁并释放。
- **输入**：场景路径或 ResourceId、加载模式（Single/Additive）、完成回调。
- **输出**：ISceneWorld、当前活动场景、与 Entity 生命周期协同。

---

## 5. 派生 ABI（与契约对齐）

- **004-scene-ABI**：loadScene、loadSceneAsync、unloadScene、getActiveScene、setActiveScene；ISceneWorld、createEntity、destroyEntity、getEntities。详见 `specs/_contracts/004-scene-ABI.md`。

---

## 6. 验收要点

- 可同步或异步加载场景；可切换当前活动场景；可卸载场景并释放 Entity 与资源。
- 场景作为 Entity 容器，加载/卸载与 Entity 生命周期一致。
