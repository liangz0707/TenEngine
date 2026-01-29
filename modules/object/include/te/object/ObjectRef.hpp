/** ObjectRef (contract: 002-object-public-api.md fullversion-001) */

#ifndef TE_OBJECT_OBJECT_REF_HPP
#define TE_OBJECT_OBJECT_REF_HPP

#include <cstdint>

namespace te::object {

/** Object reference; maps to resource GUID; resolution semantics with 013-Resource at integration. */
struct ObjectRef {
    uint8_t guid[16];
};

}  // namespace te::object

#endif
