/**
 * @file BuiltinMeshes.cpp
 * @brief 020-Pipeline: Procedural built-in meshes implementation.
 */

#include <te/pipeline/BuiltinMeshes.h>
#include <te/pipelinecore/RenderItem.h>
#include <te/mesh/MeshFactory.h>
#include <te/mesh/MeshAssetDesc.h>
#include <te/mesh/MeshResource.h>
#include <te/rendercore/resource_desc.hpp>
#include <te/core/alloc.h>
#include <cmath>
#include <memory>
#include <vector>

namespace te {
namespace pipeline {

namespace {

using namespace te::rendercore;
using namespace te::mesh;

// Fullscreen quad: 4 vertices (pos float3, uv float2), 6 indices
struct VertexPosUv {
  float x, y, z;
  float u, v;
};

static pipelinecore::IMeshHandle const* g_fullscreenQuad = nullptr;
static pipelinecore::IMeshHandle const* g_sphere = nullptr;
static float g_sphereRadius = 1.f;
static uint32_t g_sphereSegments = 16;
static pipelinecore::IMeshHandle const* g_cone = nullptr;
static float g_coneRadius = 1.f, g_coneHeight = 1.f;
static uint32_t g_coneSegments = 16;

pipelinecore::IMeshHandle const* CreateFullscreenQuadImpl() {
  VertexPosUv vertices[4] = {
    {-1.f, -1.f, 0.f, 0.f, 0.f},
    { 1.f, -1.f, 0.f, 1.f, 0.f},
    {-1.f,  1.f, 0.f, 0.f, 1.f},
    { 1.f,  1.f, 0.f, 1.f, 1.f},
  };
  uint16_t indices[6] = { 0, 1, 2, 2, 1, 3 };

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

  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinFullscreenQuad";
  desc.vertexLayout = CreateVertexFormat(vfDesc);
  desc.vertexData = vertices;
  desc.vertexDataSize = sizeof(vertices);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = indices;
  desc.indexDataSize = sizeof(indices);
  desc.submeshes.push_back(SubmeshDesc{0, 6, 0});

  if (!desc.IsValid()) return nullptr;
  MeshHandle mh = CreateMesh(&desc);
  if (!mh) return nullptr;

  MeshResource* res = new MeshResource();
  res->SetMeshHandle(mh);
  return reinterpret_cast<pipelinecore::IMeshHandle const*>(res);
}

pipelinecore::IMeshHandle const* CreateSphereImpl(float radius, uint32_t segments) {
  if (segments < 3) segments = 3;
  std::vector<float> vb;
  std::vector<uint16_t> ib;
  uint32_t rings = segments / 2;
  if (rings < 2) rings = 2;
  uint32_t sectors = segments;
  vb.reserve((rings + 1) * (sectors + 1) * 5);  // x,y,z,u,v
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
  vfDesc.stride = 20;

  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinSphere";
  desc.vertexLayout = CreateVertexFormat(vfDesc);
  desc.vertexData = vb.data();
  desc.vertexDataSize = vb.size() * sizeof(float);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = ib.data();
  desc.indexDataSize = ib.size() * sizeof(uint16_t);
  desc.submeshes.push_back(SubmeshDesc{0, static_cast<uint32_t>(ib.size()), 0});

  if (!desc.IsValid()) return nullptr;
  MeshHandle mh = CreateMesh(&desc);
  if (!mh) return nullptr;

  MeshResource* res = new MeshResource();
  res->SetMeshHandle(mh);
  return reinterpret_cast<pipelinecore::IMeshHandle const*>(res);
}

pipelinecore::IMeshHandle const* CreateConeImpl(float radius, float height, uint32_t segments) {
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
  vfDesc.stride = 20;

  MeshAssetDesc desc;
  desc.debugDescription = "BuiltinCone";
  desc.vertexLayout = CreateVertexFormat(vfDesc);
  desc.vertexData = vb.data();
  desc.vertexDataSize = vb.size() * sizeof(float);
  desc.indexFormat = CreateIndexFormat(IndexFormatDesc{IndexType::UInt16});
  desc.indexData = ib.data();
  desc.indexDataSize = ib.size() * sizeof(uint16_t);
  desc.submeshes.push_back(SubmeshDesc{0, static_cast<uint32_t>(ib.size()), 0});

  if (!desc.IsValid()) return nullptr;
  MeshHandle mh = CreateMesh(&desc);
  if (!mh) return nullptr;

  MeshResource* res = new MeshResource();
  res->SetMeshHandle(mh);
  return reinterpret_cast<pipelinecore::IMeshHandle const*>(res);
}

}  // namespace

pipelinecore::IMeshHandle const* GetFullscreenQuadMesh() {
  if (!g_fullscreenQuad)
    g_fullscreenQuad = CreateFullscreenQuadImpl();
  return g_fullscreenQuad;
}

pipelinecore::IMeshHandle const* GetSphereMesh(float radius, uint32_t segments) {
  if (!g_sphere || g_sphereRadius != radius || g_sphereSegments != segments) {
    if (g_sphere) {
      MeshResource* r = const_cast<MeshResource*>(reinterpret_cast<MeshResource const*>(g_sphere));
      delete r;
      g_sphere = nullptr;
    }
    g_sphereRadius = radius;
    g_sphereSegments = segments;
    g_sphere = CreateSphereImpl(radius, segments);
  }
  return g_sphere;
}

pipelinecore::IMeshHandle const* GetConeMesh(float radius, float height, uint32_t segments) {
  if (!g_cone || g_coneRadius != radius || g_coneHeight != height || g_coneSegments != segments) {
    if (g_cone) {
      MeshResource* r = const_cast<MeshResource*>(reinterpret_cast<MeshResource const*>(g_cone));
      delete r;
      g_cone = nullptr;
    }
    g_coneRadius = radius;
    g_coneHeight = height;
    g_coneSegments = segments;
    g_cone = CreateConeImpl(radius, height, segments);
  }
  return g_cone;
}

}  // namespace pipeline
}  // namespace te
