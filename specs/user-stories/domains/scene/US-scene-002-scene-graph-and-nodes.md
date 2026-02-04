# US-scene-002：场景图与节点（层级、父子、局部/世界变换）

- **标题**：场景由**场景图**组织：**节点**具父子关系、**局部变换**与**世界变换**、脏标记与变换更新；与 Entity 可选绑定（节点即 Entity 或节点持有 Entity）；渲染/物理以世界变换为据。
- **编号**：US-scene-002

---

## 1. 角色/触发

- **角色**：游戏逻辑、渲染、物理、Editor
- **触发**：需要**层级结构**（父子节点）、**局部/世界变换**（位置、旋转、缩放）、变换变更时**自动更新世界变换**（脏标记）；渲染与物理使用世界变换。

---

## 2. 端到端流程

1. **Scene** 提供 **节点**（Node 或与 Entity 一一对应）：每个节点有 **父节点**、**子节点列表**、**局部变换**（localPosition、localRotation、localScale）、**世界变换**（worldMatrix 或等价）。
2. 当局部变换或父节点变更时，该节点及其子节点标记为**脏**；在**更新阶段**（或提交渲染前）按拓扑顺序**更新世界变换**（父世界 × 局部 = 世界）。
3. 渲染/物理/脚本通过 **getWorldTransform** 或 **getWorldMatrix** 获取节点世界变换；与 005-Entity 的 TransformComponent 可统一或桥接。
4. 节点与 Entity 的对应关系由实现约定：一节点一 Entity、或节点为纯空间结构而 Entity 挂在其上。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 004-Scene | 场景图、节点、父子关系、局部/世界变换、脏标记、getWorldTransform、updateTransforms |
| 005-Entity | TransformComponent 与节点/变换对接（可选） |
| 020-Pipeline / 014-Physics | 消费世界变换进行剔除/绘制/物理同步 |

---

## 4. 每模块职责与 I/O

### 004-Scene

- **职责**：提供场景图结构（INode、getParent、getChildren、setLocalTransform、getWorldTransform）；脏标记与 **updateTransforms**；与 Entity 可选绑定。
- **输入**：节点创建/销毁、局部变换或父子关系变更。
- **输出**：世界变换、层级迭代；供渲染/物理使用。

---

## 5. 派生 ABI（与契约对齐）

- **004-scene-ABI**：INode、getParent、getChildren、setLocalTransform、getWorldTransform、updateTransforms；NodeId、SceneRef。详见 `specs/_contracts/004-scene-ABI.md`。

---

## 6. 验收要点

- 场景图具父子节点、局部/世界变换；变换变更后世界变换可正确更新（脏标记 + updateTransforms）。
- 渲染与物理可使用节点世界变换。
