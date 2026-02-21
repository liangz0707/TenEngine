# 026-Networking Module ABI

- **Contract**: [026-networking-public-api.md](./026-networking-public-api.md) (capabilities and type descriptions)
- **This file**: 026-Networking public ABI explicit table.
- **Status**: NOT IMPLEMENTED

> **Note**: This module is a placeholder. No implementation exists. The ABI below defines the intended symbols for future implementation.

## ABI Table

Column definitions: **Module | Namespace | Class | Export | Interface | Header | Symbol | Description**

### Connection Management

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 026-Networking | te::networking | INetworkManager | abstract interface | Connect | te/networking/NetworkManager.h | INetworkManager::Connect | `bool Connect(char const* address, uint16_t port);` Client connect; returns false on failure |
| 026-Networking | te::networking | INetworkManager | abstract interface | Disconnect | te/networking/NetworkManager.h | INetworkManager::Disconnect | `void Disconnect();` |
| 026-Networking | te::networking | INetworkManager | abstract interface | Role and Authority | te/networking/NetworkManager.h | INetworkManager::GetRole, SetAuthority | `NetworkRole GetRole() const;` `void SetAuthority(EntityId entityId, bool authoritative);` Integrates with main loop Tick |
| 026-Networking | te::networking | INetworkManager | abstract interface | Tick | te/networking/NetworkManager.h | INetworkManager::Tick | `void Tick(float deltaTime);` Called each frame by application; managed by application |
| 026-Networking | te::networking | -- | free function/singleton | Get network manager | te/networking/NetworkManager.h | GetNetworkManager | `INetworkManager* GetNetworkManager();` Provided by application or Subsystems; caller does not own pointer |

### Replication

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 026-Networking | te::networking | INetworkManager | abstract interface | Register replication | te/networking/Replication.h | INetworkManager::RegisterReplicated | `void RegisterReplicated(EntityId entityId, ReplicatedPropertyTable const* table);` Entity/component replication, property table; integrates with Entity |
| 026-Networking | te::networking | -- | free function/interface | Serialize snapshot | te/networking/Replication.h | SerializeSnapshot | `void SerializeSnapshot(IEntity* entity, ReplicatedPropertyTable const* table, void* outBuffer, size_t* outSize);` Integrates with Object serialization (optional) |
| 026-Networking | te::networking | -- | free function/interface | Apply snapshot | te/networking/Replication.h | ApplySnapshot | `void ApplySnapshot(IEntity* entity, void const* buffer, size_t size);` Bound to entity or connection |
| 026-Networking | te::networking | -- | free function/interface | Interpolate | te/networking/Replication.h | Interpolate | `void Interpolate(IEntity* entity, void const* snapshotA, void const* snapshotB, float t);` Optional |
| 026-Networking | te::networking | -- | struct | Replicated property table | te/networking/Replication.h | ReplicatedPropertyTable | Property ID, offset, type; integrates with Entity component |

### RPC

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 026-Networking | te::networking | INetworkManager | abstract interface | Register RPC | te/networking/RPC.h | INetworkManager::RegisterRPC | `void RegisterRPC(char const* name, RPCCallback callback);` Bound to connection or session |
| 026-Networking | te::networking | INetworkManager | abstract interface | Invoke RPC | te/networking/RPC.h | INetworkManager::Invoke | `void Invoke(char const* name, void const* args, size_t size, RPCTarget target);` Client/Server/Multicast |
| 026-Networking | te::networking | -- | enum | RPC target | te/networking/RPC.h | RPCTarget | Client, Server, Multicast |

### Transport (Optional)

| Module | Namespace | Class | Export | Interface | Header | Symbol | Description |
|--------|-----------|-------|--------|-----------|--------|--------|-------------|
| 026-Networking | te::networking | INetworkManager | abstract interface | Set transport | te/networking/Transport.h | INetworkManager::SetTransport | `void SetTransport(ITransport* transport);` UDP/TCP/Relay (optional) |
| 026-Networking | te::networking | ITransport | abstract interface | Send/Receive | te/networking/Transport.h | ITransport::Send, Receive | `bool Send(void const* data, size_t size);` `size_t Receive(void* buffer, size_t maxSize);` Optional |

## Change Log

| Date | Change |
|------|--------|
| T0 | 026-Networking ABI created |
| 2026-02-05 | Unified directory format |
| 2026-02-22 | Marked as NOT IMPLEMENTED; verified no header files exist in Engine/TenEngine-026-networking/ |
