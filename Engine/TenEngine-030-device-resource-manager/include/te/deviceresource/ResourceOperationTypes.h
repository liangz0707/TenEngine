/** @file ResourceOperationTypes.h
 *  030-DeviceResourceManager ABI: Resource operation status and handle types.
 */
#pragma once

#include <cstddef>

namespace te {
namespace deviceresource {

/**
 * Resource operation status enumeration.
 * Similar to LoadStatus in 013-Resource module.
 */
enum class ResourceOperationStatus {
  Pending,     // Request created, waiting to start
  Uploading,   // Currently uploading data (allocating staging buffer, recording commands)
  Submitted,  // Commands submitted to GPU, waiting for fence
  Completed,  // Operation completed successfully
  Failed,     // Operation failed
  Cancelled   // Operation was cancelled
};

/**
 * Opaque handle for resource creation operations.
 * Returned by async creation methods, used for status query and cancellation.
 */
using ResourceOperationHandle = void*;

}  // namespace deviceresource
}  // namespace te
