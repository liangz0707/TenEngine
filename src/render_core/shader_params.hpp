#pragma once
// 009-RenderCore ShaderParams API
// Contract: specs/_contracts/009-rendercore-public-api.md §1. ShaderParams

#include "types.hpp"

namespace TenEngine::RenderCore {

/// Define Uniform layout from descriptor.
/// Returns invalid layout on failure (empty handle).
/// Aligns with Shader reflection or hand-written layout.
UniformLayout DefineLayout(UniformLayoutDesc const& desc);

/// Get byte offset of member by name.
/// Returns 0 if layout invalid or member not found (per contract).
size_t GetOffset(UniformLayout layout, char const* memberName);

/// Release layout (cleanup internal resources).
void ReleaseLayout(UniformLayout& layout);

} // namespace TenEngine::RenderCore
