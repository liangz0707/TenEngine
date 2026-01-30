#pragma once
// 009-RenderCore UniformBuffer API
// Contract: specs/_contracts/009-rendercore-public-api.md §4. UniformBuffer

#include "types.hpp"

namespace TenEngine::RenderCore {

/// Create Uniform buffer from UniformLayout.
/// Returns UniformBufferHandle; invalid on failure.
/// Optional validation vs Shader convention; fails on mismatch.
UniformBufferHandle CreateUniformBuffer(UniformLayout layout);

/// Update Uniform buffer content.
/// data/size must match layout.
void Update(UniformBufferHandle handle, void const* data, size_t size);

/// Advance ring buffer slot.
/// On exhaustion: Block (block or require caller wait/retry until slot free).
/// Returns true if slot advanced; false if exhausted (caller should retry).
bool RingBufferAdvance(UniformBufferHandle handle);

/// Allocate ring buffer slot (alternative API).
/// Returns slot index; returns UINT32_MAX if exhausted (caller should wait/retry).
uint32_t RingBufferAllocSlot(UniformBufferHandle handle);

/// Bind Uniform buffer to RHI slot.
/// Aligns with Shader module and RHI binding.
void Bind(UniformBufferHandle handle, BindSlot slot);

/// Release Uniform buffer.
void ReleaseUniformBuffer(UniformBufferHandle& handle);

} // namespace TenEngine::RenderCore
