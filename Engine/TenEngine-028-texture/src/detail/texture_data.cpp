/**
 * @file texture_data.cpp
 * @brief TextureData destructor implementation.
 */
#include <te/texture/detail/texture_data.hpp>

namespace te {
namespace texture {
namespace detail {

TextureData::~TextureData() {
  /* CPU-only: no device texture stored here. */
}

}  // namespace detail
}  // namespace texture
}  // namespace te
