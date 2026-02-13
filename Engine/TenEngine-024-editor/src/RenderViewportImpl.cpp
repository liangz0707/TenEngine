/**
 * @file RenderViewportImpl.cpp
 * @brief Render viewport placeholder (024-Editor).
 */
#include <te/editor/Viewport.h>
#include <te/editor/EntityAdapter.h>

namespace te {
namespace editor {

class RenderViewportImpl : public IViewport {
public:
  IEntity* PickInViewport(int x, int y) const override { (void)x; (void)y; return nullptr; }
  void DropFromResourceManager(te::resource::ResourceId const& resourceId, int x, int y) override { (void)resourceId; (void)x; (void)y; }
  int GetWidth() const override { return m_width; }
  int GetHeight() const override { return m_height; }
  void SetSize(int w, int h) override { m_width = w; m_height = h; }
private:
  int m_width = 800, m_height = 600;
};

IViewport* CreateRenderViewport() {
  return new RenderViewportImpl();
}

}  // namespace editor
}  // namespace te
