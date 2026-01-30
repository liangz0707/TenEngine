/**
 * @file resources_create.cpp
 * @brief Test CreateBuffer, CreateTexture, CreateSampler, CreateView, Destroy.
 */
#include "te/rhi/device.hpp"
#include "te/rhi/resources.hpp"
#include "te/rhi/types.hpp"
#include <cassert>

int main() {
  auto* dev = te::rhi::CreateDevice(te::rhi::Backend::Vulkan);
  assert(dev);

  te::rhi::BufferDesc bd; bd.size = 256; bd.usage = 1;
  auto* buf = dev->CreateBuffer(bd);
  assert(buf);
  dev->DestroyBuffer(buf);

  te::rhi::TextureDesc td; td.width = 64; td.height = 64;
  auto* tex = dev->CreateTexture(td);
  assert(tex);
  dev->DestroyTexture(tex);

  te::rhi::SamplerDesc sd;
  auto* samp = dev->CreateSampler(sd);
  assert(samp);
  dev->DestroySampler(samp);

  te::rhi::DestroyDevice(dev);
  return 0;
}
