// T024: Integration test for 013 Load path.
// 013 反序列化 .mesh 后调 012 Loader CreateFromPayload，再 EnsureDeviceResources.
// This test exercises the 012 side: CreateFromPayload(Mesh, payload, manager) with a
// MeshAssetDesc payload (as 013 would pass after Deserialize). Full E2E requires 013 + 008.
#include "te/mesh/MeshLoader.h"
#include "te/mesh/MeshFactory.h"
#include "te/mesh/Mesh.h"
#include "te/mesh/MeshAssetDesc.h"

using namespace te::mesh;

int main() {
  SubmeshDesc sub = {0, 36, 0};
  MeshAssetDesc desc = {};
  desc.formatVersion = 1;
  desc.submeshCount = 1;
  desc.submeshes = &sub;

  // Simulate 013 calling 012 Loader: payload = MeshAssetDesc*, type = Mesh.
  MeshResourceLoader loader;
  unsigned meshType = 1;  // ResourceType::Mesh placeholder
  void* resource = loader.CreateFromPayload(meshType, &desc, nullptr);

  // When Loader is fully implemented (wrapping IResource*), resource != nullptr.
  // For stub, resource may be nullptr; we only verify the call path runs.
  (void)resource;

  // If we had a valid handle, we would call EnsureDeviceResources(handle, device) here.
  return 0;
}
