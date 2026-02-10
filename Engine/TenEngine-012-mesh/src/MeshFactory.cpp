/**
 * @file MeshFactory.cpp
 * @brief Mesh creation and release implementation.
 */
#include <te/mesh/MeshFactory.h>
#include <te/mesh/detail/mesh_data.hpp>
#include <te/core/alloc.h>
#include <cstring>
#include <algorithm>
#include <memory>

namespace te {
namespace mesh {

MeshHandle CreateMesh(MeshAssetDesc const* desc) {
  if (!desc || !desc->IsValid()) {
    return nullptr;
  }
  
  // Create MeshData structure
  auto data = std::make_unique<detail::MeshData>();
  if (!data) {
    return nullptr;
  }
  
  // Copy vertex layout and index format
  data->vertexLayout = desc->vertexLayout;
  data->indexFormat = desc->indexFormat;
  
  // Allocate and copy vertex data
  if (desc->vertexDataSize > 0 && desc->vertexData) {
    data->vertexDataSize = desc->vertexDataSize;
    uint8_t* vertexDataPtr = static_cast<uint8_t*>(te::core::Alloc(desc->vertexDataSize, alignof(uint8_t)));
    if (!vertexDataPtr) {
      return nullptr;
    }
    std::memcpy(vertexDataPtr, desc->vertexData, desc->vertexDataSize);
    // Use custom deleter for te::core::Alloc memory
    data->vertexData = std::unique_ptr<uint8_t[], detail::MeshData::CoreAllocDeleter>(vertexDataPtr);
  }
  
  // Allocate and copy index data
  if (desc->indexDataSize > 0 && desc->indexData) {
    data->indexDataSize = desc->indexDataSize;
    uint8_t* indexDataPtr = static_cast<uint8_t*>(te::core::Alloc(desc->indexDataSize, alignof(uint8_t)));
    if (!indexDataPtr) {
      return nullptr;
    }
    std::memcpy(indexDataPtr, desc->indexData, desc->indexDataSize);
    // Use custom deleter for te::core::Alloc memory
    data->indexData = std::unique_ptr<uint8_t[], detail::MeshData::CoreAllocDeleter>(indexDataPtr);
  }
  
  // Copy submeshes
  data->submeshes = desc->submeshes;
  
  // Copy LOD levels
  data->lodLevels = desc->lodLevels;
  
  // Copy skinning data if present
  if (desc->skinningData) {
    auto skinningData = std::make_unique<SkinningData>();
    if (!skinningData) {
      return nullptr;
    }
    
    skinningData->boneCount = desc->skinningData->boneCount;
    skinningData->vertexCount = desc->skinningData->vertexCount;
    
    // Allocate and copy bone indices
    if (desc->skinningData->boneIndicesCount > 0 && desc->skinningData->boneIndices) {
      skinningData->boneIndicesCount = desc->skinningData->boneIndicesCount;
      size_t indicesSize = skinningData->boneIndicesCount * sizeof(int32_t);
      skinningData->boneIndices = static_cast<int32_t*>(te::core::Alloc(indicesSize, alignof(int32_t)));
      if (!skinningData->boneIndices) {
        return nullptr;
      }
      std::memcpy(skinningData->boneIndices, desc->skinningData->boneIndices, indicesSize);
    }
    
    // Allocate and copy bone weights
    if (desc->skinningData->boneWeightsCount > 0 && desc->skinningData->boneWeights) {
      skinningData->boneWeightsCount = desc->skinningData->boneWeightsCount;
      size_t weightsSize = skinningData->boneWeightsCount * sizeof(float);
      skinningData->boneWeights = static_cast<float*>(te::core::Alloc(weightsSize, alignof(float)));
      if (!skinningData->boneWeights) {
        if (skinningData->boneIndices) {
          te::core::Free(skinningData->boneIndices);
        }
        return nullptr;
      }
      std::memcpy(skinningData->boneWeights, desc->skinningData->boneWeights, weightsSize);
    }
    
    // Allocate and copy bind pose matrices
    if (desc->skinningData->bindPoseMatrixCount > 0 && desc->skinningData->bindPoseMatrices) {
      skinningData->bindPoseMatrixCount = desc->skinningData->bindPoseMatrixCount;
      size_t matricesSize = skinningData->bindPoseMatrixCount * sizeof(float);
      skinningData->bindPoseMatrices = static_cast<float*>(te::core::Alloc(matricesSize, alignof(float)));
      if (!skinningData->bindPoseMatrices) {
        if (skinningData->boneIndices) {
          te::core::Free(skinningData->boneIndices);
        }
        if (skinningData->boneWeights) {
          te::core::Free(skinningData->boneWeights);
        }
        return nullptr;
      }
      std::memcpy(skinningData->bindPoseMatrices, desc->skinningData->bindPoseMatrices, matricesSize);
    }
    
    data->skinningData = std::move(skinningData);
  }
  
  // Return handle (transfer ownership)
  return data.release();
}

void ReleaseMesh(MeshHandle h) {
  if (!h) {
    return;
  }
  
  detail::MeshData* data = static_cast<detail::MeshData*>(h);
  
  // Free skinning data if present
  if (data->skinningData) {
    if (data->skinningData->boneIndices) {
      te::core::Free(data->skinningData->boneIndices);
    }
    if (data->skinningData->boneWeights) {
      te::core::Free(data->skinningData->boneWeights);
    }
    if (data->skinningData->bindPoseMatrices) {
      te::core::Free(data->skinningData->bindPoseMatrices);
    }
  }
  
  // Free vertex and index data (managed by unique_ptr with custom deleter)
  // The unique_ptr will call the deleter which uses te::core::Free
  data->vertexData.reset();
  data->indexData.reset();
  
  // Note: GPU resources (deviceVertexBuffer, deviceIndexBuffer) should be
  // destroyed by MeshResource or MeshDevice, not here.
  
  // Delete MeshData structure
  delete data;
}

}  // namespace mesh
}  // namespace te
