/**
 * @file BuiltinMeshes.cpp
 * @brief Implementation of BuiltinMeshes.
 */

#include <te/pipeline/BuiltinMeshes.h>

#include <te/rhi/device.hpp>
#include <te/rhi/resources.hpp>

#include <cmath>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <vector>

namespace te::pipeline {

// === BuiltinMeshes::Impl ===

struct CachedMesh {
  rendercore::IRenderMesh* mesh{nullptr};
  rhi::IBuffer* vertexBuffer{nullptr};
  rhi::IBuffer* indexBuffer{nullptr};
  size_t vertexCount{0};
  size_t indexCount{0};
  size_t memoryUsed{0};
};

struct BuiltinMeshes::Impl {
  rhi::IDevice* device{nullptr};
  std::unordered_map<uint32_t, CachedMesh> cache;
  std::mutex mutex;
  size_t totalMemory{0};

  uint32_t MakeKey(BuiltinMeshId id, uint32_t param1 = 0, uint32_t param2 = 0) {
    return (static_cast<uint32_t>(id) << 16) | ((param1 & 0xFF) << 8) | (param2 & 0xFF);
  }

  void Clear() {
    std::lock_guard<std::mutex> lock(mutex);
    for (auto& pair : cache) {
      if (pair.second.vertexBuffer && device) {
        device->DestroyBuffer(pair.second.vertexBuffer);
      }
      if (pair.second.indexBuffer && device) {
        device->DestroyBuffer(pair.second.indexBuffer);
      }
    }
    cache.clear();
    totalMemory = 0;
  }
};

// === BuiltinMeshes ===

BuiltinMeshes::BuiltinMeshes()
  : impl_(std::make_unique<Impl>()) {
}

BuiltinMeshes::~BuiltinMeshes() {
  impl_->Clear();
}

void BuiltinMeshes::SetDevice(rhi::IDevice* device) {
  impl_->device = device;
}

void BuiltinMeshes::GetFullscreenQuadVertices(float* outVertices, size_t stride, float* outUVs) {
  // Fullscreen quad in NDC space
  // Z = 0.5 for proper depth testing
  float vertices[] = {
    -1.0f, -1.0f, 0.5f,  // Bottom-left
     1.0f, -1.0f, 0.5f,  // Bottom-right
     1.0f,  1.0f, 0.5f,  // Top-right
    -1.0f,  1.0f, 0.5f,  // Top-left
  };

  float uvs[] = {
    0.0f, 0.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
    0.0f, 1.0f,
  };

  for (int i = 0; i < 4; ++i) {
    if (outVertices) {
      float* dst = reinterpret_cast<float*>(reinterpret_cast<char*>(outVertices) + i * stride);
      dst[0] = vertices[i * 3];
      dst[1] = vertices[i * 3 + 1];
      dst[2] = vertices[i * 3 + 2];
    }
    if (outUVs) {
      outUVs[i * 2] = uvs[i * 2];
      outUVs[i * 2 + 1] = uvs[i * 2 + 1];
    }
  }
}

void BuiltinMeshes::GetFullscreenQuadIndices(uint16_t* outIndices) {
  if (outIndices) {
    outIndices[0] = 0;
    outIndices[1] = 1;
    outIndices[2] = 2;
    outIndices[3] = 0;
    outIndices[4] = 2;
    outIndices[5] = 3;
  }
}

size_t BuiltinMeshes::GetSphereVertexCount(uint32_t segments, uint32_t rings) {
  return (segments + 1) * (rings + 1);
}

size_t BuiltinMeshes::GetSphereIndexCount(uint32_t segments, uint32_t rings) {
  return segments * rings * 6;
}

void BuiltinMeshes::GenerateSphere(
    float* outPositions, float* outNormals, float* outUVs, uint16_t* outIndices,
    uint32_t segments, uint32_t rings, float radius) {

  const float PI = 3.14159265359f;
  uint32_t vertexIndex = 0;
  uint32_t indexIndex = 0;

  // Generate vertices
  for (uint32_t ring = 0; ring <= rings; ++ring) {
    float phi = PI * static_cast<float>(ring) / static_cast<float>(rings);
    float sinPhi = std::sin(phi);
    float cosPhi = std::cos(phi);

    for (uint32_t seg = 0; seg <= segments; ++seg) {
      float theta = 2.0f * PI * static_cast<float>(seg) / static_cast<float>(segments);
      float sinTheta = std::sin(theta);
      float cosTheta = std::cos(theta);

      float x = cosTheta * sinPhi;
      float y = cosPhi;
      float z = sinTheta * sinPhi;

      if (outPositions) {
        outPositions[vertexIndex * 3] = x * radius;
        outPositions[vertexIndex * 3 + 1] = y * radius;
        outPositions[vertexIndex * 3 + 2] = z * radius;
      }

      if (outNormals) {
        outNormals[vertexIndex * 3] = x;
        outNormals[vertexIndex * 3 + 1] = y;
        outNormals[vertexIndex * 3 + 2] = z;
      }

      if (outUVs) {
        outUVs[vertexIndex * 2] = static_cast<float>(seg) / static_cast<float>(segments);
        outUVs[vertexIndex * 2 + 1] = static_cast<float>(ring) / static_cast<float>(rings);
      }

      vertexIndex++;
    }
  }

  // Generate indices
  if (outIndices) {
    for (uint32_t ring = 0; ring < rings; ++ring) {
      for (uint32_t seg = 0; seg < segments; ++seg) {
        uint32_t current = ring * (segments + 1) + seg;
        uint32_t next = current + segments + 1;

        // Two triangles per quad
        outIndices[indexIndex++] = static_cast<uint16_t>(current);
        outIndices[indexIndex++] = static_cast<uint16_t>(next);
        outIndices[indexIndex++] = static_cast<uint16_t>(current + 1);

        outIndices[indexIndex++] = static_cast<uint16_t>(current + 1);
        outIndices[indexIndex++] = static_cast<uint16_t>(next);
        outIndices[indexIndex++] = static_cast<uint16_t>(next + 1);
      }
    }
  }
}

rendercore::IRenderMesh* BuiltinMeshes::GetFullscreenQuad() {
  uint32_t key = impl_->MakeKey(BuiltinMeshId::FullscreenQuad);

  std::lock_guard<std::mutex> lock(impl_->mutex);

  auto it = impl_->cache.find(key);
  if (it != impl_->cache.end()) {
    return it->second.mesh;
  }

  // Create fullscreen quad
  CachedMesh cached;

  // Vertex data: position (3) + uv (2)
  struct Vertex {
    float x, y, z;
    float u, v;
  };

  Vertex vertices[4];
  GetFullscreenQuadVertices(
    reinterpret_cast<float*>(vertices), sizeof(Vertex),
    reinterpret_cast<float*>(&vertices[0].u));

  uint16_t indices[6];
  GetFullscreenQuadIndices(indices);

  // Create buffers
  if (impl_->device) {
    rhi::BufferDesc vbDesc{};
    vbDesc.size = sizeof(vertices);
    vbDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex);
    cached.vertexBuffer = impl_->device->CreateBuffer(vbDesc);
    if (cached.vertexBuffer) {
      impl_->device->UpdateBuffer(cached.vertexBuffer, 0, vertices, sizeof(vertices));
    }

    rhi::BufferDesc ibDesc{};
    ibDesc.size = sizeof(indices);
    ibDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index);
    cached.indexBuffer = impl_->device->CreateBuffer(ibDesc);
    if (cached.indexBuffer) {
      impl_->device->UpdateBuffer(cached.indexBuffer, 0, indices, sizeof(indices));
    }
  }

  cached.vertexCount = 4;
  cached.indexCount = 6;
  cached.memoryUsed = sizeof(vertices) + sizeof(indices);
  impl_->totalMemory += cached.memoryUsed;

  impl_->cache[key] = cached;
  return cached.mesh;
}

rendercore::IRenderMesh* BuiltinMeshes::GetSphere(uint32_t segments) {
  uint32_t key = impl_->MakeKey(BuiltinMeshId::Sphere32, segments);

  std::lock_guard<std::mutex> lock(impl_->mutex);

  auto it = impl_->cache.find(key);
  if (it != impl_->cache.end()) {
    return it->second.mesh;
  }

  uint32_t rings = segments / 2;
  size_t vertexCount = GetSphereVertexCount(segments, rings);
  size_t indexCount = GetSphereIndexCount(segments, rings);

  // Allocate and generate
  std::vector<float> positions(vertexCount * 3);
  std::vector<float> normals(vertexCount * 3);
  std::vector<float> uvs(vertexCount * 2);
  std::vector<uint16_t> indices(indexCount);

  GenerateSphere(
    positions.data(), normals.data(), uvs.data(), indices.data(),
    segments, rings, 1.0f);

  // Create vertex buffer (interleaved)
  struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
  };

  std::vector<Vertex> vertices(vertexCount);
  for (size_t i = 0; i < vertexCount; ++i) {
    vertices[i].x = positions[i * 3];
    vertices[i].y = positions[i * 3 + 1];
    vertices[i].z = positions[i * 3 + 2];
    vertices[i].nx = normals[i * 3];
    vertices[i].ny = normals[i * 3 + 1];
    vertices[i].nz = normals[i * 3 + 2];
    vertices[i].u = uvs[i * 2];
    vertices[i].v = uvs[i * 2 + 1];
  }

  CachedMesh cached;

  if (impl_->device) {
    rhi::BufferDesc vbDesc{};
    vbDesc.size = vertices.size() * sizeof(Vertex);
    vbDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex);
    cached.vertexBuffer = impl_->device->CreateBuffer(vbDesc);
    if (cached.vertexBuffer) {
      impl_->device->UpdateBuffer(cached.vertexBuffer, 0,
        vertices.data(), vbDesc.size);
    }

    rhi::BufferDesc ibDesc{};
    ibDesc.size = indices.size() * sizeof(uint16_t);
    ibDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index);
    cached.indexBuffer = impl_->device->CreateBuffer(ibDesc);
    if (cached.indexBuffer) {
      impl_->device->UpdateBuffer(cached.indexBuffer, 0,
        indices.data(), ibDesc.size);
    }
  }

  cached.vertexCount = vertexCount;
  cached.indexCount = indexCount;
  cached.memoryUsed = vertices.size() * sizeof(Vertex) + indices.size() * sizeof(uint16_t);
  impl_->totalMemory += cached.memoryUsed;

  impl_->cache[key] = cached;
  return cached.mesh;
}

rendercore::IRenderMesh* BuiltinMeshes::GetCone(float height, float radius) {
  // TODO: Implement cone generation
  return nullptr;
}

rendercore::IRenderMesh* BuiltinMeshes::GetConeTruncated(float height, float radius) {
  // TODO: Implement truncated cone generation
  return nullptr;
}

rendercore::IRenderMesh* BuiltinMeshes::GetCube() {
  uint32_t key = impl_->MakeKey(BuiltinMeshId::Cube);

  std::lock_guard<std::mutex> lock(impl_->mutex);

  auto it = impl_->cache.find(key);
  if (it != impl_->cache.end()) {
    return it->second.mesh;
  }

  // Cube vertices (position + normal + uv)
  struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
  };

  // 24 vertices (4 per face with proper normals)
  Vertex vertices[] = {
    // Front face
    {-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f},
    { 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f, 1.0f},
    // Back face
    { 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f},
    {-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f},
    {-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f},
    { 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f},
    // Top face
    {-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f},
    { 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f},
    // Bottom face
    {-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f},
    { 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f},
    {-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f},
    // Right face
    { 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f},
    { 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f},
    { 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f},
    { 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f},
    // Left face
    {-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f},
    {-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f},
    {-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f},
    {-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f},
  };

  uint16_t indices[] = {
    // Front
    0, 1, 2, 0, 2, 3,
    // Back
    4, 5, 6, 4, 6, 7,
    // Top
    8, 9, 10, 8, 10, 11,
    // Bottom
    12, 13, 14, 12, 14, 15,
    // Right
    16, 17, 18, 16, 18, 19,
    // Left
    20, 21, 22, 20, 22, 23,
  };

  CachedMesh cached;

  if (impl_->device) {
    rhi::BufferDesc vbDesc{};
    vbDesc.size = sizeof(vertices);
    vbDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Vertex);
    cached.vertexBuffer = impl_->device->CreateBuffer(vbDesc);
    if (cached.vertexBuffer) {
      impl_->device->UpdateBuffer(cached.vertexBuffer, 0, vertices, sizeof(vertices));
    }

    rhi::BufferDesc ibDesc{};
    ibDesc.size = sizeof(indices);
    ibDesc.usage = static_cast<uint32_t>(rhi::BufferUsage::Index);
    cached.indexBuffer = impl_->device->CreateBuffer(ibDesc);
    if (cached.indexBuffer) {
      impl_->device->UpdateBuffer(cached.indexBuffer, 0, indices, sizeof(indices));
    }
  }

  cached.vertexCount = 24;
  cached.indexCount = 36;
  cached.memoryUsed = sizeof(vertices) + sizeof(indices);
  impl_->totalMemory += cached.memoryUsed;

  impl_->cache[key] = cached;
  return cached.mesh;
}

rendercore::IRenderMesh* BuiltinMeshes::GetBox(float sizeX, float sizeY, float sizeZ) {
  // For now, just return unit cube - scaling handled in shader
  return GetCube();
}

rendercore::IRenderMesh* BuiltinMeshes::GetCylinder(float height, float radius) {
  // TODO: Implement cylinder generation
  return nullptr;
}

rendercore::IRenderMesh* BuiltinMeshes::GetCapsule(float height, float radius) {
  // TODO: Implement capsule generation
  return nullptr;
}

rendercore::IRenderMesh* BuiltinMeshes::GetPlane(uint32_t subdivisions) {
  // TODO: Implement plane generation
  return nullptr;
}

rendercore::IRenderMesh* BuiltinMeshes::GetMesh(BuiltinMeshId id, BuiltinMeshParams const* params) {
  switch (id) {
    case BuiltinMeshId::FullscreenQuad:
      return GetFullscreenQuad();
    case BuiltinMeshId::Sphere16:
    case BuiltinMeshId::Sphere32:
    case BuiltinMeshId::Sphere64:
      return GetSphere(params ? params->segments : 32);
    case BuiltinMeshId::Cone:
      return GetCone(params ? params->height : 1.0f, params ? params->radius : 1.0f);
    case BuiltinMeshId::ConeTruncated:
      return GetConeTruncated(params ? params->height : 1.0f, params ? params->radius : 1.0f);
    case BuiltinMeshId::Cube:
    case BuiltinMeshId::Box:
      return GetCube();
    case BuiltinMeshId::Cylinder:
      return GetCylinder(params ? params->height : 1.0f, params ? params->radius : 1.0f);
    case BuiltinMeshId::Capsule:
      return GetCapsule(params ? params->height : 1.0f, params ? params->radius : 0.5f);
    default:
      return nullptr;
  }
}

bool BuiltinMeshes::IsCached(BuiltinMeshId id) const {
  std::lock_guard<std::mutex> lock(impl_->mutex);
  return impl_->cache.find(static_cast<uint32_t>(id)) != impl_->cache.end();
}

void BuiltinMeshes::ClearCache() {
  impl_->Clear();
}

size_t BuiltinMeshes::GetMemoryUsed() const {
  return impl_->totalMemory;
}

// === Global Instance ===

static std::unique_ptr<BuiltinMeshes> g_builtinMeshes;
static std::mutex g_builtinMutex;

BuiltinMeshes* GetBuiltinMeshes() {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  if (!g_builtinMeshes) {
    g_builtinMeshes = std::make_unique<BuiltinMeshes>();
  }
  return g_builtinMeshes.get();
}

void InitializeBuiltinMeshes(rhi::IDevice* device) {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  if (!g_builtinMeshes) {
    g_builtinMeshes = std::make_unique<BuiltinMeshes>();
  }
  g_builtinMeshes->SetDevice(device);
}

void ShutdownBuiltinMeshes() {
  std::lock_guard<std::mutex> lock(g_builtinMutex);
  g_builtinMeshes.reset();
}

// === Free Functions ===

BuiltinMeshes* CreateBuiltinMeshes() {
  return new BuiltinMeshes();
}

void DestroyBuiltinMeshes(BuiltinMeshes* meshes) {
  delete meshes;
}

}  // namespace te::pipeline
