/**
 * @file ResourceView.h
 * @brief Resource browser view (ABI IResourceView).
 */
#ifndef TE_EDITOR_RESOURCE_VIEW_H
#define TE_EDITOR_RESOURCE_VIEW_H

#include <functional>
#include <string>
#include <vector>

namespace te {
namespace resource {
class IResourceManager;
}  // namespace resource
namespace editor {

class IResourceView {
public:
  virtual ~IResourceView() = default;
  virtual void OnDraw() = 0;
  virtual void SetRootPath(char const* path) = 0;
  /** Callback when user double-clicks a .level file to open. */
  virtual void SetOnOpenLevel(std::function<void(std::string const&)> fn) = 0;
  /** Callback when user requests delete of a level file (path to .level). */
  virtual void SetOnDeleteLevel(std::function<void(std::string const&)> fn) = 0;
  /** Set resource manager for Import (right-click / drag-drop). */
  virtual void SetResourceManager(te::resource::IResourceManager* manager) = 0;
  /** Import files (e.g. from OS drag-drop). Target dir = current selected directory. */
  virtual void ImportFiles(std::vector<std::string> const& paths) = 0;
};

IResourceView* CreateResourceView();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_RESOURCE_VIEW_H
