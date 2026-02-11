/**
 * @file BuiltinMeshes.h
 * @brief Built-in procedural meshes (te::mesh, same level as MeshResource). Cached; do not delete returned pointers.
 */
#ifndef TE_MESH_BUILTIN_MESHES_H
#define TE_MESH_BUILTIN_MESHES_H

namespace te {
namespace mesh {

class MeshResource;

/** Fullscreen quad (NDC -1..1, UV 0..1); for post-process. Cached singleton. */
MeshResource const* GetFullscreenQuadMesh();

/** Sphere. Cached per (radius, segments). Do not delete. */
MeshResource const* GetSphereMesh(float radius = 1.f, uint32_t segments = 16);

/** Hemisphere (half sphere, Y >= 0). Cached per (radius, segments). */
MeshResource const* GetHemisphereMesh(float radius = 1.f, uint32_t segments = 16);

/** Plane on XY, centered at origin. width (X) and height (Y). Cached per (width, height). */
MeshResource const* GetPlaneMesh(float width = 1.f, float height = 1.f);

/** Quad (alias for plane). Same as GetPlaneMesh. */
inline MeshResource const* GetQuadMesh(float width = 1.f, float height = 1.f) {
  return GetPlaneMesh(width, height);
}

/** Single triangle (3 vertices, 3 indices). Cached singleton. */
MeshResource const* GetTriangleMesh();

/** Cube, centered at origin. size = half-extent per axis. Cached per size. */
MeshResource const* GetCubeMesh(float size = 1.f);

/** Cone (e.g. spot light volume). Cached per (radius, height, segments). */
MeshResource const* GetConeMesh(float radius = 1.f, float height = 1.f, uint32_t segments = 16);

}  // namespace mesh
}  // namespace te

#endif  // TE_MESH_BUILTIN_MESHES_H
