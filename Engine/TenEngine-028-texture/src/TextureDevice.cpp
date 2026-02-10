/**
 * @file TextureDevice.cpp
 * @brief GPU device resource management for texture implementation.
 */
#include <te/texture/TextureDevice.h>
#include <te/texture/Texture.h>
#include <te/texture/detail/texture_data.hpp>
#include <te/deviceresource/DeviceResourceManager.h>
#include <te/rhi/resources.hpp>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/log.h>
#include <cstdint>
#include <new>

namespace te {
namespace texture {

namespace {

/** Map 009-RenderCore TextureFormat to 008-RHI format (uint32_t). Backend uses 0 for default RGBA8_UNORM. */
uint32_t TextureFormatToRHI(rendercore::TextureFormat format) {
  switch (format) {
    case rendercore::TextureFormat::RGBA8_UNorm:
    case rendercore::TextureFormat::RGBA8_SRGB:
    case rendercore::TextureFormat::BGRA8_UNorm:
      /* Use 0 so 008 backends use default R8G8B8A8_UNORM; upload data is already in correct layout. */
      return 0;
    case rendercore::TextureFormat::R8_UNorm:
    case rendercore::TextureFormat::RG8_UNorm:
    case rendercore::TextureFormat::R16_Float:
    case rendercore::TextureFormat::RG16_Float:
    case rendercore::TextureFormat::RGBA16_Float:
    case rendercore::TextureFormat::R32_Float:
    case rendercore::TextureFormat::RG32_Float:
    case rendercore::TextureFormat::RGBA32_Float:
      return static_cast<uint32_t>(format);
    default:
      return 0;
  }
}

struct AsyncTextureContext {
  TextureHandle h;
  void (*on_done)(void*);
  void* user_data;
};

void OnDeviceTextureCreated(rhi::ITexture* texture, bool success, void* user_data) {
  AsyncTextureContext* ctx = static_cast<AsyncTextureContext*>(user_data);
  if (!ctx) return;
  if (success && texture && ctx->h) {
    static_cast<detail::TextureData*>(ctx->h)->deviceTexture = texture;
  }
  if (ctx->on_done) {
    ctx->on_done(ctx->user_data);
  }
  delete ctx;
}

}  // namespace

bool EnsureDeviceResources(TextureHandle h, rhi::IDevice* device) {
  if (!h || !device) {
    return false;
  }

  detail::TextureData* data = static_cast<detail::TextureData*>(h);
  if (data->deviceTexture) {
    return true;
  }

  void const* pixels = data->pixelData.get();
  size_t size = data->pixelDataSize;
  if (!pixels || size == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDevice: No pixel data for EnsureDeviceResources");
    return false;
  }

  rhi::TextureDesc rhiDesc{};
  rhiDesc.width = data->width;
  rhiDesc.height = data->height;
  rhiDesc.depth = data->depth;
  rhiDesc.format = TextureFormatToRHI(data->format);

  rhi::ITexture* tex = deviceresource::DeviceResourceManager::CreateDeviceTexture(
      pixels, size, rhiDesc, device);
  if (!tex) {
    return false;
  }

  data->deviceTexture = tex;
  return true;
}

void EnsureDeviceResourcesAsync(TextureHandle h, rhi::IDevice* device,
                                void (*on_done)(void*), void* user_data) {
  if (!h || !device) {
    if (on_done) on_done(user_data);
    return;
  }

  detail::TextureData* data = static_cast<detail::TextureData*>(h);
  if (data->deviceTexture) {
    if (on_done) on_done(user_data);
    return;
  }

  void const* pixels = data->pixelData.get();
  size_t size = data->pixelDataSize;
  if (!pixels || size == 0) {
    te::core::Log(te::core::LogLevel::Error, "TextureDevice: No pixel data for EnsureDeviceResourcesAsync");
    if (on_done) on_done(user_data);
    return;
  }

  rhi::TextureDesc rhiDesc{};
  rhiDesc.width = data->width;
  rhiDesc.height = data->height;
  rhiDesc.depth = data->depth;
  rhiDesc.format = TextureFormatToRHI(data->format);

  AsyncTextureContext* ctx = new (std::nothrow) AsyncTextureContext{h, on_done, user_data};
  if (!ctx) {
    te::core::Log(te::core::LogLevel::Error, "TextureDevice: Failed to allocate async context");
    if (on_done) on_done(user_data);
    return;
  }

  deviceresource::DeviceResourceManager::CreateDeviceTextureAsync(
      pixels, size, rhiDesc, device, OnDeviceTextureCreated, ctx);
}

rhi::ITexture* GetTextureHandle(TextureHandle h) {
  if (!h) return nullptr;
  return static_cast<detail::TextureData*>(h)->deviceTexture;
}

void DestroyDeviceTexture(TextureHandle h, rhi::IDevice* device) {
  if (!h || !device) return;
  detail::TextureData* data = static_cast<detail::TextureData*>(h);
  if (data->deviceTexture) {
    deviceresource::DeviceResourceManager::DestroyDeviceTexture(data->deviceTexture, device);
    data->deviceTexture = nullptr;
  }
}

}  // namespace texture
}  // namespace te
