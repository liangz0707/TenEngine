# Data Model: 009-RenderCore

**Feature**: 009-render-core-shader-reflection-abi  
**Date**: 2026-02-03

## 实体与类型

### 1. UniformLayoutDesc / UniformMember

- **UniformMember**：单成员描述；`name[64]`、`UniformMemberType type`、`offset`、`size`
- **UniformLayoutDesc**：`members`（UniformMember 数组）、`memberCount`、`totalSize`（0 表示自动计算）
- **验证**：memberCount > 0；members 非空；type != Unknown；offset/size 可由 0 表示自动计算

### 2. VertexFormatDesc / VertexFormat

- **VertexFormatDesc**：`attributes`、`attributeCount`、`stride`
- **VertexFormat**：`attributes[]`、`attributeCount`、`stride`；IsValid() = attributeCount>0 && stride>0

### 3. IndexFormatDesc / IndexFormat

- **IndexFormatDesc**：`IndexType type`
- **IndexFormat**：`type`；IsValid() = type != Unknown

### 4. TextureDescParams / TextureDesc

- **TextureDescParams**：width, height, depth, mipLevels, format, usage
- **TextureDesc**：同上；IsValid() = width>0 && height>0 && format!=Unknown

### 5. BufferDescParams / BufferDesc

- **BufferDescParams**：size, usage, alignment
- **BufferDesc**：同上；IsValid() = size>0

### 6. PassResourceDecl

- **PassResourceDecl**：pass, resource, isRead, isWrite, lifetime
- **验证**：pass.IsValid() && resource.IsValid()

### 7. 句柄类型（不透明）

- **IUniformLayout**：CreateUniformLayout 返回；GetOffset、GetTotalSize
- **IUniformBuffer**：CreateUniformBuffer 返回；Update、Bind、GetRingBufferOffset、SetCurrentFrameSlot

## 状态与生命周期

- **UniformLayout**：CreateUniformLayout 创建；ReleaseUniformLayout 释放
- **UniformBuffer**：CreateUniformBuffer 创建；ReleaseUniformBuffer 释放；RingBuffer 多帧 slot 由 SetCurrentFrameSlot 管理
- **PassResourceDecl**：单次 Pass 图构建周期；DeclareRead/DeclareWrite 产出

## 与 010-Shader 反射格式约定

- **UniformMemberType** 与 SPIR-V 基础类型映射：float→Float, vec2→Float2, vec3→Float3, vec4→Float4, mat3→Mat3, mat4→Mat4, int→Int 等
- **偏移**：按 std140 规则；010-Shader 产出时填充 offset/size，或填 0 由 009 CreateUniformLayout 计算
