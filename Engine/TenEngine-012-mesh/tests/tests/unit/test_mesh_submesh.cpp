// T018: Unit test for mesh submesh API; calls 012-mesh public API (and 001 alloc via CreateMesh).
#include "te/mesh/MeshFactory.h"
#include "te/mesh/Mesh.h"
#include "te/mesh/MeshAssetDesc.h"

using namespace te::mesh;

int main() {
  SubmeshDesc sub[2] = {
    { 0, 36, 0 },
    { 36, 24, 1 },
  };
  MeshAssetDesc desc = {};
  desc.formatVersion = 1;
  desc.submeshCount = 2;
  desc.submeshes = sub;

  MeshHandle h = CreateMesh(&desc);
  if (!h.valid()) return 1;
  if (GetSubmeshCount(h) != 2) return 2;
  SubmeshDesc const* s0 = GetSubmesh(h, 0);
  SubmeshDesc const* s1 = GetSubmesh(h, 1);
  if (!s0 || s0->offset != 0 || s0->count != 36 || s0->materialSlotIndex != 0) return 3;
  if (!s1 || s1->offset != 36 || s1->count != 24 || s1->materialSlotIndex != 1) return 4;
  if (GetSubmesh(h, 2) != nullptr) return 5;  // out of range
  ReleaseMesh(h);
  return 0;
}
