#pragma once
// 009-RenderCore Public API
// Contract: specs/_contracts/009-rendercore-public-api.md
//
// This header aggregates only contract-declared types and functions.
// No RHI internals are exposed.
//
// Initialization Order:
//   Core and RHI MUST be initialized before using this API (per FR-006).
//
// Upstream Contracts:
//   - 001-Core: specs/_contracts/001-core-public-api.md
//   - 008-RHI:  specs/_contracts/008-rhi-public-api.md

#include "types.hpp"
#include "resource_desc.hpp"
#include "shader_params.hpp"
#include "pass_protocol.hpp"
#include "uniform_buffer.hpp"

// ============================================================================
// TenEngine::RenderCore Public API Summary
// ============================================================================
//
// 1. ShaderParams (shader_params.hpp)
//    - DefineLayout(UniformLayoutDesc const&) -> UniformLayout
//    - GetOffset(UniformLayout, char const*) -> size_t
//    - ReleaseLayout(UniformLayout&)
//
// 2. ResourceDesc (resource_desc.hpp)
//    - CreateVertexFormat(VertexFormatDesc const&) -> VertexFormat
//    - CreateIndexFormat(IndexFormatDesc const&) -> IndexFormat
//    - CreateTextureDesc(TextureDescParams const&) -> TextureDesc
//    - CreateBufferDesc(BufferDescParams const&) -> BufferDesc
//
// 3. PassProtocol (pass_protocol.hpp)
//    - DeclareRead(PassHandle, ResourceHandle) -> PassResourceDecl
//    - DeclareWrite(PassHandle, ResourceHandle) -> PassResourceDecl
//    - SetResourceLifetime(PassResourceDecl&, ResourceLifetime)
//
// 4. UniformBuffer (uniform_buffer.hpp)
//    - CreateUniformBuffer(UniformLayout) -> UniformBufferHandle
//    - Update(UniformBufferHandle, void const*, size_t)
//    - RingBufferAdvance(UniformBufferHandle) -> bool
//    - RingBufferAllocSlot(UniformBufferHandle) -> uint32_t
//    - Bind(UniformBufferHandle, BindSlot)
//    - ReleaseUniformBuffer(UniformBufferHandle&)
//
// Validation Behavior (per spec clarification):
//   - CreateUniformBuffer/Bind MAY validate layout vs Shader convention.
//   - On mismatch, returns error (invalid handle).
//   - Validation behavior is optional and documented.
//
// RingBuffer Exhaustion (per spec clarification):
//   - RingBufferAdvance returns false if exhausted; caller should wait/retry.
//   - RingBufferAllocSlot returns UINT32_MAX if exhausted; caller should wait/retry.
//   - No silent overwrite of in-flight data.
//
// ============================================================================
