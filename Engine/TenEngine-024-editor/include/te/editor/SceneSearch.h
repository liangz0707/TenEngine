/**
 * @file SceneSearch.h
 * @brief Scene search and filter interface.
 */
#ifndef TE_EDITOR_SCENE_SEARCH_H
#define TE_EDITOR_SCENE_SEARCH_H

#include <te/editor/EditorTypes.h>
#include <te/entity/EntityId.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace te {
namespace editor {

/**
 * @brief Search filter criteria for scene hierarchy.
 */
struct SceneSearchFilter {
  const char* namePattern = nullptr;     ///< Name pattern (supports wildcards)
  bool caseSensitive = false;
  bool searchComponents = false;          ///< Also search component names
  bool searchInactive = true;             ///< Include inactive entities
  int componentTypeFilter = -1;           ///< -1 for all types
  int layerFilter = -1;                   ///< -1 for all layers
  int tagFilter = -1;                     ///< -1 for all tags
};

/**
 * @brief Scene search manager interface.
 * 
 * Provides search and filtering capabilities for the scene hierarchy.
 */
class ISceneSearch {
public:
  virtual ~ISceneSearch() = default;
  
  // === Search Operations ===
  
  /**
   * @brief Perform a search with the current filter.
   * @return Number of results found
   */
  virtual size_t Search() = 0;
  
  /**
   * @brief Set the search filter.
   */
  virtual void SetFilter(SceneSearchFilter const& filter) = 0;
  
  /**
   * @brief Get the current filter.
   */
  virtual SceneSearchFilter const& GetFilter() const = 0;
  
  /**
   * @brief Clear search results.
   */
  virtual void ClearResults() = 0;
  
  // === Results ===
  
  /**
   * @brief Get search results.
   */
  virtual std::vector<te::entity::EntityId> const& GetResults() const = 0;
  
  /**
   * @brief Get result count.
   */
  virtual size_t GetResultCount() const = 0;
  
  /**
   * @brief Check if entity matches current filter.
   */
  virtual bool MatchesFilter(te::entity::EntityId id) const = 0;
  
  // === History ===
  
  /**
   * @brief Get search history.
   */
  virtual std::vector<const char*> const& GetHistory() const = 0;
  
  /**
   * @brief Clear search history.
   */
  virtual void ClearHistory() = 0;
  
  /**
   * @brief Set maximum history size.
   */
  virtual void SetMaxHistorySize(size_t size) = 0;
  
  // === Callbacks ===
  
  /**
   * @brief Set callback for search completion.
   */
  virtual void SetOnSearchComplete(std::function<void()> callback) = 0;
  
  // === Drawing ===
  
  /**
   * @brief Draw search UI.
   */
  virtual void OnDraw() = 0;
  
  /**
   * @brief Check if search bar is focused.
   */
  virtual bool IsSearchFocused() const = 0;
  
  /**
   * @brief Focus the search bar.
   */
  virtual void FocusSearch() = 0;
};

/**
 * @brief Factory function to create scene search instance.
 */
ISceneSearch* CreateSceneSearch();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_SCENE_SEARCH_H
