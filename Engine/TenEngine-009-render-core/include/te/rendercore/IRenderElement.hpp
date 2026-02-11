/** @file IRenderElement.hpp
 *  009-RenderCore: Single draw resource set = IRenderMesh + IRenderMaterial.
 */
#pragma once

namespace te {
namespace rendercore {

struct IRenderMesh;
struct IRenderMaterial;

/** Aggregates mesh and material for one draw; stored by pointer. */
struct IRenderElement {
  virtual ~IRenderElement() = default;
  virtual IRenderMesh* GetMesh() = 0;
  virtual IRenderMesh const* GetMesh() const = 0;
  virtual IRenderMaterial* GetMaterial() = 0;
  virtual IRenderMaterial const* GetMaterial() const = 0;
};

}  // namespace rendercore
}  // namespace te
