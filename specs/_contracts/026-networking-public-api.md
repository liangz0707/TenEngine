# 契约：026-Networking 模块对外 API

## 适用模块

- **实现方**：026-Networking（L4；复制、RPC、客户端/服务端；无下游）
- **对应规格**：`docs/module-specs/026-networking.md`
- **依赖**：001-Core、005-Entity

## 消费者

- 无（L4 消费端；向游戏逻辑提供 NetworkManager、Replication、RPC、Connection）

## 能力列表

### 类型与句柄（跨边界）

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| NetworkManager | 连接管理、角色、权威、与主循环 Tick 对接 | 由应用管理 |
| ReplicationHandle | 实体/组件复制、属性表、SerializeSnapshot、ApplySnapshot、Interpolate | 与实体或连接绑定 |

### 能力（提供方保证）

| 序号 | 能力 | 说明 |
|------|------|------|
| 1 | 连接 | NetworkManager、连接管理、角色、权威 |
| 2 | 复制 | ReplicationHandle、SerializeSnapshot、ApplySnapshot、Interpolate；与 Entity 对接 |
| 3 | RPC | 远程调用接口；与主循环 Tick 对接 |

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。

## 约束

- 须在 Core、Entity 初始化之后使用。若未来 Editor 需网络状态展示可在此扩展。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 026-Networking 契约 |
| 2026-02-05 | 统一目录；能力列表用表格 |
