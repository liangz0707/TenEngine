/**
 * @file ResourceId.h
 * @brief Resource global unique ID / GUID (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_ID_H
#define TE_RESOURCE_RESOURCE_ID_H

#include <cstdint>

namespace te {
namespace resource {

/** Opaque resource ID; equivalent to GUID for FResource references and addressing. */
struct ResourceId {
  uint64_t data[2] = {0, 0};

  bool operator==(ResourceId const& o) const { return data[0] == o.data[0] && data[1] == o.data[1]; }
  bool operator!=(ResourceId const& o) const { return !(*this == o); }
  bool operator<(ResourceId const& o) const {
    return data[0] != o.data[0] ? data[0] < o.data[0] : data[1] < o.data[1];
  }
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_ID_H
