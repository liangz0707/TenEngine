/** @file Guid.hpp
 *  ABI: specs/_contracts/002-object-ABI.md
 */
#ifndef TE_OBJECT_GUID_HPP
#define TE_OBJECT_GUID_HPP

#include <cstdint>

namespace te {
namespace object {

struct GUID {
  uint8_t data[16] = {};
};

}  // namespace object
}  // namespace te

#endif  // TE_OBJECT_GUID_HPP
