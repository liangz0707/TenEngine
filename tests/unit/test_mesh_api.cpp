// T023: Unit tests for CreateMesh, ReleaseMesh, GetLODCount, SelectLOD, GetSkinningData.
// Uses 012-mesh public API; te_mesh links 001-core (Alloc/Free used inside CreateMesh).
#include "te/mesh/MeshFactory.h"
#include "te/mesh/Mesh.h"
#include "te/mesh/MeshAssetDesc.h"

using namespace te::mesh;

static int test_create_null_desc() {
  MeshHandle h = CreateMesh(nullptr);
  if (h.valid()) return 1;
  return 0;
}

static int test_release() {
  SubmeshDesc sub = {0, 12, 0};
  MeshAssetDesc desc = {};
  desc.formatVersion = 1;
  desc.submeshCount = 1;
  desc.submeshes = &sub;
  MeshHandle h = CreateMesh(&desc);
  if (!h.valid()) return 1;
  ReleaseMesh(h);
  return 0;
}

static int test_lod_count_and_select() {
  SubmeshDesc sub = {0, 36, 0};
  MeshAssetDesc desc = {};
  desc.formatVersion = 1;
  desc.submeshCount = 1;
  desc.submeshes = &sub;
  desc.lodCount = 3;
  MeshHandle h = CreateMesh(&desc);
  if (!h.valid()) return 1;
  if (GetLODCount(h) != 3) return 2;
  if (SelectLOD(h, 0.f) != 0) return 3;
  if (SelectLOD(h, 500.f) != 2) return 4;  // clip to last LOD
  ReleaseMesh(h);
  return 0;
}

static int test_skinning_null() {
  MeshAssetDesc desc = {};
  desc.formatVersion = 1;
  desc.submeshCount = 0;
  MeshHandle h = CreateMesh(&desc);
  if (!h.valid()) return 1;
  if (GetSkinningData(h) != nullptr) return 2;  // no skinning
  ReleaseMesh(h);
  return 0;
}

static int test_skinning_present() {
  SkinningData skin = {};
  skin.boneCount = 4;
  SubmeshDesc sub = {0, 24, 0};
  MeshAssetDesc desc = {};
  desc.formatVersion = 1;
  desc.submeshCount = 1;
  desc.submeshes = &sub;
  desc.skinningData = &skin;
  MeshHandle h = CreateMesh(&desc);
  if (!h.valid()) return 1;
  SkinningData const* s = GetSkinningData(h);
  if (!s || s->boneCount != 4) return 2;
  ReleaseMesh(h);
  return 0;
}

int main() {
  if (test_create_null_desc() != 0) return 10;
  if (test_release() != 0) return 20;
  if (test_lod_count_and_select() != 0) return 30;
  if (test_skinning_null() != 0) return 40;
  if (test_skinning_present() != 0) return 50;
  return 0;
}
