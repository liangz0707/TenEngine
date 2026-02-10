/**
 * Unit tests for TextureResource: construction, Release, GetResourceType, GetResourceId.
 */
#include <te/texture/TextureResource.h>
#include <te/resource/ResourceTypes.h>
#include <cassert>

int main() {
  using namespace te::texture;

  TextureResource res;
  assert(res.GetResourceType() == te::resource::ResourceType::Texture);
  assert(!res.GetResourceId().IsNull());
  res.Release();
  return 0;
}
