/**
 * @file TextureModule.cpp
 * @brief TextureModule implementation (CPU-only).
 */
#include <te/texture/TextureModule.h>

namespace te {
namespace texture {

TextureModule& TextureModule::GetInstance() {
  static TextureModule s_instance;
  return s_instance;
}

}  // namespace texture
}  // namespace te
