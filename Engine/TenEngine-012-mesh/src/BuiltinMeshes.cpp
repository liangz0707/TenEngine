/**
 * @file BuiltinMeshes.cpp
 * @brief Built-in procedural meshes implementation (012-Mesh).
 */
#include <te/mesh/BuiltinMeshes.h>
#include <te/mesh/MeshResource.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/rendercore/resource_desc.hpp>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <unordered_map>
#include <vector>

namespace te {
namespace mesh {

namespace {

using namespace te::rendercore;

struct VertexPosUv {
  float x, y, z;
  float u, v;
};

static MeshResource const* g_fullscreenQuad = nullptr;
static MeshResource const* g_triangle = nullptr;

struct SphereKey {
  float radius;
  uint32_t segments;
  bool operator==(SphereKey const& o) const {
    return radius == o.radius && segments == o.segments;
  }
};
struct SphereKeyHash {
  size_t operator()(SphereKey const& k) const {
    return std::hash<float>{}(k.radius) ^ (std::hash<uint32_t>{}(k.segments) << 1);
  }
};
static std::unordered_map<SphereKey, std::unique_ptr<MeshResource>, SphereKeyHash> g_sphereCache;

struct HemisphereKey {
  float radius;
  uint32_t segments;
  bool operator==(HemisphereKey const& o) const {
    return radius == o.radius && segments == o.segments;
  }
};
struct HemisphereKeyHash {
  size_t operator()(HemisphereKey const& k) const {
    return std::hash<float>{}(k.radius) ^ (std::hash<uint32_t>{}(k.segments) << 1);
  }
};
static std::unordered_map<HemisphereKey, std::unique_ptr<MeshResource>, HemisphereKeyHash> g_hemisphereCache;

struct PlaneKey {
  float width, height;
  bool operator==(PlaneKey const& o) const { return width == o.width && height == o.height; }
};
struct PlaneKeyHash {
  size_t operator()(PlaneKey const& k) const {
    return std::hash<float>{}(k.width) ^ (std::hash<float>{}(k.height) << 1);
  }
};
static std::unordered_map<PlaneKey, std::unique_ptr<MeshResource>, PlaneKeyHash> g_planeCache;

static std::unordered_map<float, std::unique_ptr<MeshResource>> g_cubeCache;

struct ConeKey {
  float radius, height;
  uint32_t segments;
  bool operator==(ConeKey const& o) const {
    return radius == o.radius && height == o.height && segments == o.segments;
  }
};
struct ConeKeyHash {
  size_t operator()(ConeKey const& k) const {
    return std::hash<float>{}(k.radius) ^ (std::hash<float>{}(k.height) << 1) ^
           (std::hash<uint32_t>{}(k.segments) << 2);
  }
};
static std::unordered_map<ConeKey, std::unique_ptr<MeshResource>, ConeKeyHash> g_coneCache;

VertexFormat MakePosUvFormat() {
  VertexAttribute attrs[2] = {};
  attrs[0].location = 0;
  attrs[0].format = VertexAttributeFormat::Float3;
  attrs[0].offset = 0;
  attrs[1].location = 1;
  attrs[1].format = VertexAttributeFormat::Float2;
  attrs[1].offset = 12;
  VertexFormatDesc vfDesc;
  vfDesc.attributes = attrs;
  vfDesc.attributeCount = 2;
  vfDesc.stride = sizeof(VertexPosUv);
  return CreateVertexFormat(vfDesc);
}

MeshResource* CreateFromDesc(MeshAssetDesc& desc) {
  if (!desc.IsValid()) return nullptr;
  MeshHandle mh = CreateMesh(&desc);
  if (!mh) return nullptr;
  MeshResource* res = new MeshResource();
  res->SetMeshHandle(mh);
  return res;
}

MeshResource const* CreateFullscreenQuadImpl() {
  VertexPosUv vertices[4] = {
    {-1.f, -1.f, 0.f, 0.f, 0.f},
    { 1.f, -1.f, 0.f, 1.f, 0.f},
    {-1.f,  1.f, 0.f, 0.f, 1.f},
    { 1.f,  1.f, 0.f, 1.f, 1.f},
  };
  uint16_t indices[6] = {0, 1, 2, 2, 1, 3};
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinFullscreenQuad";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vertices;
  desc.vertexDataSize = sizeof(vertices);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = indices;
  desc.indexDataSize = sizeof(indices);
  desc.submeshes.push_back(SubmeshDesc{0, 6, 0});
  return CreateFromDesc(desc);
}

MeshResource* CreateSphereImpl(float radius, uint32_t segments) {
  if (segments < 3) segments = 3;
  std::vector<float> vb;
  std::vector<uint16_t> ib;
  uint32_t rings = segments / 2;
  if (rings < 2) rings = 2;
  uint32_t sectors = segments;
  vb.reserve((rings + 1) * (sectors + 1) * 5);
  for (uint32_t r = 0; r <= rings; ++r) {
    float v = static_cast<float>(r) / static_cast<float>(rings);
    float phi = v * 3.14159265f;
    float y = std::cos(phi) * radius;
    float ringRadius = std::sin(phi) * radius;
    for (uint32_t s = 0; s <= sectors; ++s) {
      float u = static_cast<float>(s) / static_cast<float>(sectors);
      float theta = u * 2.f * 3.14159265f;
      float x = ringRadius * std::cos(theta);
      float z = ringRadius * std::sin(theta);
      vb.push_back(x); vb.push_back(y); vb.push_back(z);
      vb.push_back(u); vb.push_back(v);
    }
  }
  ib.reserve(rings * sectors * 6);
  for (uint32_t r = 0; r < rings; ++r) {
    for (uint32_t s = 0; s < sectors; ++s) {
      uint32_t a = r * (sectors + 1) + s;
      uint32_t b = a + sectors + 1;
      ib.push_back(static_cast<uint16_t>(a));
      ib.push_back(static_cast<uint16_t>(b));
      ib.push_back(static_cast<uint16_t>(a + 1));
      ib.push_back(static_cast<uint16_t>(a + 1));
      ib.push_back(static_cast<uint16_t>(b));
      ib.push_back(static_cast<uint16_t>(b + 1));
    }
  }
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinSphere";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vb.data();
  desc.vertexDataSize = vb.size() * sizeof(float);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = ib.data();
  desc.indexDataSize = ib.size() * sizeof(uint16_t);
  desc.submeshes.push_back(SubmeshDesc{0, static_cast<uint32_t>(ib.size()), 0});
  return CreateFromDesc(desc);
}

MeshResource* CreateHemisphereImpl(float radius, uint32_t segments) {
  if (segments < 3) segments = 3;
  std::vector<float> vb;
  std::vector<uint16_t> ib;
  uint32_t rings = segments / 2;
  if (rings < 1) rings = 1;
  uint32_t sectors = segments;
  for (uint32_t r = 0; r <= rings; ++r) {
    float v = static_cast<float>(r) / static_cast<float>(rings);
    float phi = v * 0.5f * 3.14159265f;
    float y = std::cos(phi) * radius;
    float ringRadius = std::sin(phi) * radius;
    for (uint32_t s = 0; s <= sectors; ++s) {
      float u = static_cast<float>(s) / static_cast<float>(sectors);
      float theta = u * 2.f * 3.14159265f;
      float x = ringRadius * std::cos(theta);
      float z = ringRadius * std::sin(theta);
      vb.push_back(x); vb.push_back(y); vb.push_back(z);
      vb.push_back(u); vb.push_back(v);
    }
  }
  for (uint32_t r = 0; r < rings; ++r) {
    for (uint32_t s = 0; s < sectors; ++s) {
      uint32_t a = r * (sectors + 1) + s;
      uint32_t b = a + sectors + 1;
      ib.push_back(static_cast<uint16_t>(a));
      ib.push_back(static_cast<uint16_t>(b));
      ib.push_back(static_cast<uint16_t>(a + 1));
      ib.push_back(static_cast<uint16_t>(a + 1));
      ib.push_back(static_cast<uint16_t>(b));
      ib.push_back(static_cast<uint16_t>(b + 1));
    }
  }
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinHemisphere";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vb.data();
  desc.vertexDataSize = vb.size() * sizeof(float);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = ib.data();
  desc.indexDataSize = ib.size() * sizeof(uint16_t);
  desc.submeshes.push_back(SubmeshDesc{0, static_cast<uint32_t>(ib.size()), 0});
  return CreateFromDesc(desc);
}

MeshResource* CreatePlaneImpl(float width, float height) {
  float hw = width * 0.5f, hh = height * 0.5f;
  VertexPosUv vertices[4] = {
    {-hw, -hh, 0.f, 0.f, 0.f},
    { hw, -hh, 0.f, 1.f, 0.f},
    {-hw,  hh, 0.f, 0.f, 1.f},
    { hw,  hh, 0.f, 1.f, 1.f},
  };
  uint16_t indices[6] = {0, 1, 2, 2, 1, 3};
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinPlane";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vertices;
  desc.vertexDataSize = sizeof(vertices);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = indices;
  desc.indexDataSize = sizeof(indices);
  desc.submeshes.push_back(SubmeshDesc{0, 6, 0});
  return CreateFromDesc(desc);
}

MeshResource* CreateTriangleImpl() {
  VertexPosUv vertices[3] = {
    {0.f, 1.f, 0.f, 0.5f, 0.f},
    {-1.f, -1.f, 0.f, 0.f, 1.f},
    {1.f, -1.f, 0.f, 1.f, 1.f},
  };
  uint16_t indices[3] = {0, 1, 2};
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinTriangle";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vertices;
  desc.vertexDataSize = sizeof(vertices);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = indices;
  desc.indexDataSize = sizeof(indices);
  desc.submeshes.push_back(SubmeshDesc{0, 3, 0});
  return CreateFromDesc(desc);
}

MeshResource* CreateCubeImpl(float size) {
  float s = size * 0.5f;
  float positions[8 * 3] = {
    -s, -s, -s,  s, -s, -s,  s, s, -s, -s, s, -s,
    -s, -s,  s,  s, -s,  s,  s, s,  s, -s, s,  s,
  };
  float uvs[8 * 2] = {
    0,0, 1,0, 1,1, 0,1, 0,0, 1,0, 1,1, 0,1,
  };
  std::vector<float> vb;
  std::vector<uint16_t> ib;
  int faces[6][4] = {{0,1,2,3}, {5,4,7,6}, {4,0,3,7}, {1,5,6,2}, {4,5,1,0}, {3,2,6,7}};
  for (int f = 0; f < 6; ++f) {
    for (int i = 0; i < 4; ++i) {
      int vi = faces[f][i];
      vb.push_back(positions[vi*3]); vb.push_back(positions[vi*3+1]); vb.push_back(positions[vi*3+2]);
      vb.push_back(uvs[vi*2]); vb.push_back(uvs[vi*2+1]);
    }
    uint16_t base = static_cast<uint16_t>(f * 4);
    ib.push_back(base); ib.push_back(base+1); ib.push_back(base+2);
    ib.push_back(base); ib.push_back(base+2); ib.push_back(base+3);
  }
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinCube";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vb.data();
  desc.vertexDataSize = vb.size() * sizeof(float);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = ib.data();
  desc.indexDataSize = ib.size() * sizeof(uint16_t);
  desc.submeshes.push_back(SubmeshDesc{0, static_cast<uint32_t>(ib.size()), 0});
  return CreateFromDesc(desc);
}

MeshResource* CreateConeImpl(float radius, float height, uint32_t segments) {
  if (segments < 3) segments = 3;
  std::vector<float> vb;
  std::vector<uint16_t> ib;
  vb.push_back(0.f); vb.push_back(height); vb.push_back(0.f); vb.push_back(0.5f); vb.push_back(0.f);
  for (uint32_t s = 0; s <= segments; ++s) {
    float t = static_cast<float>(s) / static_cast<float>(segments);
    float theta = t * 2.f * 3.14159265f;
    float x = radius * std::cos(theta);
    float z = radius * std::sin(theta);
    vb.push_back(x); vb.push_back(0.f); vb.push_back(z);
    vb.push_back(t); vb.push_back(1.f);
  }
  for (uint32_t s = 0; s < segments; ++s) {
    ib.push_back(0);
    ib.push_back(static_cast<uint16_t>(s + 1));
    ib.push_back(static_cast<uint16_t>(s + 2));
  }
  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinCone";
  desc.vertexLayout = MakePosUvFormat();
  desc.vertexData = vb.data();
  desc.vertexDataSize = vb.size() * sizeof(float);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = ib.data();
  desc.indexDataSize = ib.size() * sizeof(uint16_t);
  desc.submeshes.push_back(SubmeshDesc{0, static_cast<uint32_t>(ib.size()), 0});
  return CreateFromDesc(desc);
}

}  // namespace

MeshResource const* GetFullscreenQuadMesh() {
  if (!g_fullscreenQuad)
    g_fullscreenQuad = CreateFullscreenQuadImpl();
  return g_fullscreenQuad;
}

MeshResource const* GetSphereMesh(float radius, uint32_t segments) {
  SphereKey key{radius, segments};
  auto it = g_sphereCache.find(key);
  if (it != g_sphereCache.end())
    return it->second.get();
  auto res = std::unique_ptr<MeshResource>(CreateSphereImpl(radius, segments));
  MeshResource const* p = res.get();
  g_sphereCache[key] = std::move(res);
  return p;
}

MeshResource const* GetHemisphereMesh(float radius, uint32_t segments) {
  HemisphereKey key{radius, segments};
  auto it = g_hemisphereCache.find(key);
  if (it != g_hemisphereCache.end())
    return it->second.get();
  auto res = std::unique_ptr<MeshResource>(CreateHemisphereImpl(radius, segments));
  MeshResource const* p = res.get();
  g_hemisphereCache[key] = std::move(res);
  return p;
}

MeshResource const* GetPlaneMesh(float width, float height) {
  PlaneKey key{width, height};
  auto it = g_planeCache.find(key);
  if (it != g_planeCache.end())
    return it->second.get();
  auto res = std::unique_ptr<MeshResource>(CreatePlaneImpl(width, height));
  MeshResource const* p = res.get();
  g_planeCache[key] = std::move(res);
  return p;
}

MeshResource const* GetTriangleMesh() {
  if (!g_triangle)
    g_triangle = CreateTriangleImpl();
  return g_triangle;
}

MeshResource const* GetCubeMesh(float size) {
  auto it = g_cubeCache.find(size);
  if (it != g_cubeCache.end())
    return it->second.get();
  auto res = std::unique_ptr<MeshResource>(CreateCubeImpl(size));
  MeshResource const* p = res.get();
  g_cubeCache[size] = std::move(res);
  return p;
}

MeshResource const* GetConeMesh(float radius, float height, uint32_t segments) {
  ConeKey key{radius, height, segments};
  auto it = g_coneCache.find(key);
  if (it != g_coneCache.end())
    return it->second.get();
  auto res = std::unique_ptr<MeshResource>(CreateConeImpl(radius, height, segments));
  MeshResource const* p = res.get();
  g_coneCache[key] = std::move(res);
  return p;
}

}  // namespace mesh
}  // namespace te
