# 026-Networking 模块 ABI

- **契约**：[026-networking-public-api.md](./026-networking-public-api.md)（能力与类型描述）
- **本文件**：026-Networking 对外 ABI 显式表。
- **参考**：Unity Netcode、UE 复制/RPC；连接管理、实体/组件复制、RPC、与主循环 Tick 对接。
- **命名**：成员方法采用 **PascalCase**；说明列给出**完整函数签名**。

## ABI 表

列定义：**模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明**

### 连接管理（Connection）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 连接 | te/networking/NetworkManager.h | INetworkManager::Connect | `bool Connect(char const* address, uint16_t port);` 客户端连接；失败返回 false |
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 断开 | te/networking/NetworkManager.h | INetworkManager::Disconnect | `void Disconnect();` |
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 角色与权威 | te/networking/NetworkManager.h | INetworkManager::GetRole, SetAuthority | `NetworkRole GetRole() const;` `void SetAuthority(EntityId entityId, bool authoritative);` 与主循环 Tick 对接 |
| 026-Networking | te::networking | INetworkManager | 抽象接口 | Tick | te/networking/NetworkManager.h | INetworkManager::Tick | `void Tick(float deltaTime);` 由应用每帧调用；由应用管理 |
| 026-Networking | te::networking | — | 自由函数/单例 | 获取网络管理器 | te/networking/NetworkManager.h | GetNetworkManager | `INetworkManager* GetNetworkManager();` 由应用或 Subsystems 提供；调用方不拥有指针 |

### 复制（Replication）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 注册复制 | te/networking/Replication.h | INetworkManager::RegisterReplicated | `void RegisterReplicated(EntityId entityId, ReplicatedPropertyTable const* table);` 实体/组件复制、属性表；与 Entity 对接 |
| 026-Networking | te::networking | — | 自由函数/接口 | 序列化快照 | te/networking/Replication.h | SerializeSnapshot | `void SerializeSnapshot(IEntity* entity, ReplicatedPropertyTable const* table, void* outBuffer, size_t* outSize);` 与 Object 序列化（可选）对接 |
| 026-Networking | te::networking | — | 自由函数/接口 | 应用快照 | te/networking/Replication.h | ApplySnapshot | `void ApplySnapshot(IEntity* entity, void const* buffer, size_t size);` 与实体或连接绑定 |
| 026-Networking | te::networking | — | 自由函数/接口 | 插值 | te/networking/Replication.h | Interpolate | `void Interpolate(IEntity* entity, void const* snapshotA, void const* snapshotB, float t);` 可选 |
| 026-Networking | te::networking | — | struct | 复制属性表 | te/networking/Replication.h | ReplicatedPropertyTable | 属性 ID、偏移、类型；与 Entity 组件对接 |

### RPC

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 注册 RPC | te/networking/RPC.h | INetworkManager::RegisterRPC | `void RegisterRPC(char const* name, RPCCallback callback);` 与连接或会话绑定 |
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 调用 RPC | te/networking/RPC.h | INetworkManager::Invoke | `void Invoke(char const* name, void const* args, size_t size, RPCTarget target);` Client/Server/Multicast |
| 026-Networking | te::networking | — | 枚举 | RPC 目标 | te/networking/RPC.h | RPCTarget | Client、Server、Multicast |

### 传输（可选）

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 026-Networking | te::networking | INetworkManager | 抽象接口 | 设置传输 | te/networking/Transport.h | INetworkManager::SetTransport | `void SetTransport(ITransport* transport);` UDP/TCP/Relay（可选） |
| 026-Networking | te::networking | ITransport | 抽象接口 | 发送/接收 | te/networking/Transport.h | ITransport::Send, Receive | `bool Send(void const* data, size_t size);` `size_t Receive(void* buffer, size_t maxSize);` 可选 |

*来源：契约能力 Replication、RPC、Connection、Transport；参考 Unity Netcode、UE 复制/RPC。*
