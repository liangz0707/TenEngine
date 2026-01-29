/** GUID (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_GUID_HPP
#define TE_OBJECT_GUID_HPP

#include <cstddef>
#include <cstdint>

namespace te::object {

/** Global unique identifier; 16 bytes (contract: uint8_t data[16]). */
struct GUID {
    uint8_t data[16];
};

}  // namespace te::object

#endif
