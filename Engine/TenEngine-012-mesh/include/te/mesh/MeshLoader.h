/** 012-Mesh: MeshResourceLoader (IResourceLoader); CreateFromPayload. */
#ifndef TE_MESH_MESH_LOADER_H
#define TE_MESH_MESH_LOADER_H

namespace te {
namespace mesh {

/** 013 按 ResourceType::Mesh 调用；payload 为 MeshAssetDesc*，返回 IResource*。 */
class MeshResourceLoader {
 public:
  /** type==Mesh 时 payload 为 MeshAssetDesc*；返回 IResource*（实现 IMeshResource）。 */
  void* CreateFromPayload(unsigned type, void* payload, void* manager);
};

}  // namespace mesh
}  // namespace te

#endif
