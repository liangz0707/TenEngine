# US-networking-001：连接与 RPC（客户端/服务端连接、RPC 调用）

- **标题**：应用可建立**客户端/服务端**网络连接、发起 **RPC**（远程过程调用）；连接生命周期与 Application 或游戏会话协同；RPC 参数与返回值可序列化（与 002-Object 可选对接）。
- **编号**：US-networking-001

---

## 1. 角色/触发

- **角色**：游戏逻辑
- **触发**：需要**连接**到服务器或**接受**客户端连接；需要**调用远程方法**（RPC）并获取结果或回调；连接断开时可重连或清理。

---

## 2. 端到端流程

1. 调用方调用 **connect(host, port)** 或服务端 **listen(port)**、**accept()**；Networking 模块建立连接，返回 **ConnectionId** 或连接句柄。
2. 调用方**注册 RPC**（如 **registerRpc(name, handler)**）；对端调用 **invokeRpc(connectionId, name, args)**；Networking 序列化参数、发送、对端反序列化并调用 handler，可选返回结果或回调。
3. 连接断开时触发 **onDisconnect** 回调；调用方可 **disconnect**、**reconnect**；与 Application 生命周期或游戏会话协同。
4. RPC 参数/返回值序列化可由 002-Object 或自定义协议提供；本模块仅约定「可序列化」与调用语义。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 026-Networking | connect、listen、accept、disconnect、reconnect；registerRpc、invokeRpc；ConnectionId、onDisconnect |
| 003-Application | 可选：连接生命周期与主循环/会话协同 |
| 002-Object | 可选：RPC 参数/返回值序列化 |

---

## 4. 每模块职责与 I/O

### 026-Networking

- **职责**：提供 **connect**、**listen**、**accept**、**disconnect**、**reconnect**；**registerRpc**、**invokeRpc**；ConnectionId、onConnect、onDisconnect；与底层网络 API 抽象对接。
- **输入**：host/port、RPC 名与参数、handler。
- **输出**：连接状态、RPC 调用结果或回调；供游戏逻辑使用。

---

## 5. 派生 ABI（与契约对齐）

- **026-networking-ABI**：connect、listen、accept、disconnect、registerRpc、invokeRpc、ConnectionId。详见 `specs/_contracts/026-networking-ABI.md`。

---

## 6. 验收要点

- 可建立客户端/服务端连接、断开、重连；可注册并调用 RPC，参数可序列化传递。
