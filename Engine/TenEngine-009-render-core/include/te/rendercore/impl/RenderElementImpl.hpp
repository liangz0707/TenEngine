/** @file RenderElementImpl.hpp
 *  009-RenderCore: IRenderElement implementation; aggregates mesh + material by pointer.
 */
#pragma once

#include <te/rendercore/IRenderElement.hpp>

namespace te {
namespace rendercore {

class RenderElementImpl : public IRenderElement {
 public:
  RenderElementImpl(IRenderMesh* mesh, IRenderMaterial* material);
  IRenderMesh* GetMesh() override;
  IRenderMesh const* GetMesh() const override;
  IRenderMaterial* GetMaterial() override;
  IRenderMaterial const* GetMaterial() const override;

 private:
  IRenderMesh* mesh_;
  IRenderMaterial* material_;
};

}  // namespace rendercore
}  // namespace te
