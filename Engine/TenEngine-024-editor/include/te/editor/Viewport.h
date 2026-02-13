/**
 * @file Viewport.h
 * @brief Render viewport (ABI IViewport).
 */
#ifndef TE_EDITOR_VIEWPORT_H
#define TE_EDITOR_VIEWPORT_H

#include "EntityAdapter.h"
#include <te/resource/ResourceId.h>

namespace te {
namespace editor {

class IViewport {
public:
  virtual ~IViewport() = default;
  virtual IEntity* PickInViewport(int x, int y) const = 0;
  virtual void DropFromResourceManager(te::resource::ResourceId const& resourceId, int x, int y) = 0;
  virtual int GetWidth() const = 0;
  virtual int GetHeight() const = 0;
  virtual void SetSize(int w, int h) = 0;
};

IViewport* CreateRenderViewport();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_VIEWPORT_H
