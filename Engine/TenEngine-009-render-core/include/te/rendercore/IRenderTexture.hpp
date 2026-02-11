/** @file IRenderTexture.hpp
 *  009-RenderCore: GPU texture abstraction (sampled or attachment).
 *  028-Texture is CPU-only and does not implement this; optional implementation elsewhere.
 */
#pragma once

#include <te/rendercore/resource_desc.hpp>

namespace te {
namespace rhi {
struct ITexture;
}
namespace rendercore {

/** GPU-side texture: sampled image or render target attachment. No 013/028 refs. */
struct IRenderTexture {
  virtual ~IRenderTexture() = default;
  /** Underlying RHI texture for descriptor set, bind, destroy. */
  virtual rhi::ITexture* GetRHITexture() const = 0;
  /** Usage hint: Sampled, RenderTarget, DepthStencil, etc. */
  virtual TextureUsage GetUsage() const = 0;
  /** True if used as color/depth attachment (RenderTarget or DepthStencil). */
  inline bool IsAttachment() const {
    TextureUsage u = GetUsage();
    return (static_cast<std::uint32_t>(u) & (static_cast<std::uint32_t>(TextureUsage::RenderTarget) |
                                              static_cast<std::uint32_t>(TextureUsage::DepthStencil))) != 0;
  }
};

}  // namespace rendercore
}  // namespace te
