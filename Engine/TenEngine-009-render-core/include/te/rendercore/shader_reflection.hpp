/** @file shader_reflection.hpp
 *  009-RenderCore ABI: ShaderResourceKind, ShaderResourceBinding, ShaderReflectionDesc.
 *  Full shader reflection: Uniform buffer, Texture (SampledImage), Sampler.
 */
#pragma once

#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/types.hpp>
#include <cstddef>
#include <cstdint>

namespace te {
namespace rendercore {

/** Shader resource kinds beyond Uniform buffer members. */
enum class ShaderResourceKind : uint8_t {
  UniformBuffer,
  SampledImage,   // Texture (sampled in shader)
  Sampler,
  StorageBuffer,
  StorageImage,
  Unknown
};

/** Single resource binding: Texture or Sampler at (set, binding). */
struct ShaderResourceBinding {
  char name[64] = {};
  ShaderResourceKind kind = ShaderResourceKind::Unknown;
  uint32_t set = 0;
  uint32_t binding = 0;
};

/** Full shader reflection: Uniform block + resource bindings (Texture, Sampler). */
struct ShaderReflectionDesc {
  /** Uniform buffer block layout (constant block). May be null/empty if no UBO. */
  UniformLayoutDesc uniformBlock = {};

  /** Texture and Sampler bindings. Null if none. */
  ShaderResourceBinding const* resourceBindings = nullptr;
  uint32_t resourceBindingCount = 0;
};

}  // namespace rendercore
}  // namespace te
