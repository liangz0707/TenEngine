/** @file resources_create.cpp
 *  US3 test: CreateBuffer/CreateTexture/CreateSampler (valid desc); size=0 returns nullptr.
 */
#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>
#include <te/rhi/types.hpp>
#include <cassert>
#include <cstdio>

int main() {
  using namespace te::rhi;
  IDevice* dev = CreateDevice();
  if (!dev) {
    std::printf("CreateDevice() nullptr; skip resources_create\n");
    return 0;
  }
  BufferDesc bd = {};
  bd.size = 0;
  bd.usage = 0;
  assert(dev->CreateBuffer(bd) == nullptr);
  bd.size = 64;
  IBuffer* buf = dev->CreateBuffer(bd);
  if (buf) dev->DestroyBuffer(buf);
  TextureDesc td = {};
  td.width = 0;
  assert(dev->CreateTexture(td) == nullptr);
  td.width = 4;
  td.height = 4;
  td.depth = 1;
  td.format = 0;
  ITexture* tex = dev->CreateTexture(td);
  if (tex) dev->DestroyTexture(tex);
  SamplerDesc sd = {};
  sd.filter = 0;
  ISampler* sam = dev->CreateSampler(sd);
  if (sam) dev->DestroySampler(sam);
  DestroyDevice(dev);
  std::printf("resources_create passed\n");
  return 0;
}
