/** @file raytracing.hpp
 *  008-RHI ABI: RaytracingAccelerationStructureDesc, DispatchRaysDesc (D3D12).
 */
#pragma once

#include <te/rhi/types.hpp>
#include <te/rhi/resources.hpp>
#include <cstdint>
#include <cstddef>

namespace te {
namespace rhi {

struct RaytracingAccelerationStructureDesc {
  uint32_t type;  // BLAS/TLAS
  void*    geometryDesc;
  size_t   geometryCount;
};

struct DispatchRaysDesc {
  IBuffer* raygenShaderTable;
  size_t   raygenSize;
  size_t   raygenStride;
  IBuffer* missShaderTable;
  size_t   missSize;
  size_t   missStride;
  IBuffer* hitGroupTable;
  size_t   hitGroupSize;
  size_t   hitGroupStride;
  uint32_t width;
  uint32_t height;
  uint32_t depth;
};

}  // namespace rhi
}  // namespace te
