/**
 * @file ResourceView.h
 * @brief Resource browser view (ABI IResourceView).
 */
#ifndef TE_EDITOR_RESOURCE_VIEW_H
#define TE_EDITOR_RESOURCE_VIEW_H

namespace te {
namespace editor {

class IResourceView {
public:
  virtual ~IResourceView() = default;
  virtual void OnDraw() = 0;
  virtual void SetRootPath(char const* path) = 0;
};

IResourceView* CreateResourceView();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_RESOURCE_VIEW_H
