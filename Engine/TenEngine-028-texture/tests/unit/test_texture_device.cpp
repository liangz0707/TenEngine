/**
 * Unit tests for TextureDevice: EnsureDeviceResources is not run without a real device;
 * we only test GetTextureHandle and DestroyDeviceTexture with null/invalid handle.
 */
#include <te/texture/TextureDevice.h>
#include <te/texture/Texture.h>
#include <cassert>

int main() {
  using namespace te::texture;

  assert(GetTextureHandle(nullptr) == nullptr);
  DestroyDeviceTexture(nullptr, nullptr);
  return 0;
}
