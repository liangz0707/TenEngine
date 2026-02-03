# 契约：026-Networking 模块对外 API

## 适用模块

- **实现方**：**026-Networking**（复制、RPC 与客户端/服务端）
- **对应规格**：`docs/module-specs/026-networking.md`
- **依赖**：001-Core（001-core-public-api）、005-Entity（005-entity-public-api）

## 消费者（T0 下游）

- **无**（Networking 为 L4 消费端；向游戏逻辑提供 NetworkManager、Replication、RPC、Connection）

## 版本 / ABI

- 遵循 Constitution：公开 API 版本化；破坏性变更递增 MAJOR。
- 当前契约版本：（由实现或计划阶段填写）

## 类型与句柄（跨边界）

本模块向**游戏逻辑/应用层**提供以下类型（引擎内无其他模块依赖本模块；若未来 Editor 需网络状态展示可在此扩展）：

| 名称 | 语义 | 生命周期 |
|------|------|----------|
| NetworkManager | 连接管理、角色、权威、与主循环 Tick 对接 | 由应用管理 |
| ReplicationHandle | 实体/组件复制、属性表、SerializeSnapshot、ApplySnapshot、Interpolate | 与实体或连接绑定 |
| RPC 接口 | RegisterRPC、Invoke、Client/Server/Multicast | 与连接或会话绑定 |
| Connection 状态 | Connect、Disconnect、GetRole、SetAuthority、Tick | 由 Networking 管理 |

与 Entity 实体 ID、组件快照、复制属性表对接；与 Object 序列化（可选）对接。**ABI 显式表**：[026-networking-ABI.md](./026-networking-ABI.md)。

## 能力列表（提供方保证）

1. **Replication**：INetworkManager::RegisterReplicated；SerializeSnapshot、ApplySnapshot、Interpolate；ReplicatedPropertyTable；与 Entity 对接。
2. **RPC**：INetworkManager::RegisterRPC、Invoke；RPCTarget（Client/Server/Multicast）。
3. **Connection**：INetworkManager::Connect、Disconnect、GetRole、SetAuthority、Tick；GetNetworkManager；由应用管理。
4. **Transport（可选）**：INetworkManager::SetTransport；ITransport::Send、Receive；UDP/TCP/Relay。

## 调用顺序与约束

- 须在 Core、Entity 初始化之后使用；复制与 Entity 组件、RPC 与序列化约定一致。

## 变更记录

| 日期 | 变更说明 |
|------|----------|
| T0 新增 | 每模块一契约：026-Networking 对应本契约；无下游；与 docs/module-specs/026-networking.md 一致 |
| 2026-01-28 | 根据 026-networking-ABI 反向更新：INetworkManager、GetNetworkManager、RegisterReplicated、SerializeSnapshot、ApplySnapshot、Interpolate、RegisterRPC、Invoke、Connect、Disconnect、Tick、SetTransport；能力与类型与 ABI 表一致 |
