/** @file RenderElementImpl.cpp */
#include <te/rendercore/impl/RenderElementImpl.hpp>

namespace te {
namespace rendercore {

RenderElementImpl::RenderElementImpl(IRenderMesh* mesh, IRenderMaterial* material)
    : mesh_(mesh), material_(material) {}

IRenderMesh* RenderElementImpl::GetMesh() {
  return mesh_;
}

IRenderMesh const* RenderElementImpl::GetMesh() const {
  return mesh_;
}

IRenderMaterial* RenderElementImpl::GetMaterial() {
  return material_;
}

IRenderMaterial const* RenderElementImpl::GetMaterial() const {
  return material_;
}

}  // namespace rendercore
}  // namespace te
