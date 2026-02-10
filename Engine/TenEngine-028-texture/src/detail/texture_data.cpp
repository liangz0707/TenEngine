/**
 * @file texture_data.cpp
 * @brief TextureData destructor implementation.
 */
#include <te/texture/detail/texture_data.hpp>
#include <te/rhi/device.hpp>

namespace te {
namespace texture {
namespace detail {

TextureData::~TextureData() {
  /* GPU texture (deviceTexture) is destroyed by TextureResource or TextureDevice
   * via 030::DestroyDeviceTexture; we do not destroy it here to avoid requiring
   * IDevice* in this destructor. */
}

}  // namespace detail
}  // namespace texture
}  // namespace te
