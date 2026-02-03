/** @file types.hpp
 *  009-RenderCore ABI: ResultCode, PassHandle, ResourceHandle, FrameSlotId,
 *  ResourceLifetime, BindSlot.
 */
#pragma once

#include <cstdint>
#include <cstddef>

namespace te {
namespace rendercore {

enum class ResultCode : uint32_t {
  Success = 0,
  InvalidHandle,
  UnsupportedFormat,
  UnsupportedSize,
  ValidationFailed,
  RingBufferExhausted,
  Unknown
};

struct PassHandle {
  uint64_t id = 0;
  bool IsValid() const { return id != 0; }
};

struct ResourceHandle {
  uint64_t id = 0;
  bool IsValid() const { return id != 0; }
};

using FrameSlotId = uint32_t;

enum class ResourceLifetime : uint8_t {
  Transient,
  Persistent,
  External
};

struct BindSlot {
  uint32_t set = 0;
  uint32_t binding = 0;
};

}  // namespace rendercore
}  // namespace te
