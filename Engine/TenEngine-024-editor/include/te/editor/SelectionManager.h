/**
 * @file SelectionManager.h
 * @brief Selection management interface (ABI ISelectionManager).
 */
#ifndef TE_EDITOR_SELECTION_MANAGER_H
#define TE_EDITOR_SELECTION_MANAGER_H

#include <te/editor/EditorTypes.h>
#include <te/entity/EntityId.h>
#include <te/core/math.h>
#include <functional>
#include <vector>

namespace te {
namespace editor {

// Math type aliases for convenience
namespace math {
using Vec3 = te::core::Vector3;
}

// Note: SelectionChangeEvent and SelectionFilter are defined in EditorTypes.h

/**
 * @brief Selection manager interface.
 *
 * Manages the current selection state for the editor, including
 * multi-selection, selection events, and selection-based operations.
 */
class ISelectionManager {
public:
  virtual ~ISelectionManager() = default;

  // === Selection Operations ===

  /**
   * @brief Select a single entity (clears previous selection).
   */
  virtual void Select(te::entity::EntityId id) = 0;
  
  /**
   * @brief Select multiple entities (clears previous selection).
   */
  virtual void SelectMultiple(std::vector<te::entity::EntityId> const& ids) = 0;
  
  /**
   * @brief Add entity to current selection.
   */
  virtual void AddToSelection(te::entity::EntityId id) = 0;
  
  /**
   * @brief Toggle entity selection state.
   */
  virtual void ToggleSelection(te::entity::EntityId id) = 0;
  
  /**
   * @brief Remove entity from selection.
   */
  virtual void Deselect(te::entity::EntityId id) = 0;
  
  /**
   * @brief Clear all selection.
   */
  virtual void ClearSelection() = 0;
  
  // === Selection Query ===
  
  /**
   * @brief Check if entity is selected.
   */
  virtual bool IsSelected(te::entity::EntityId id) const = 0;
  
  /**
   * @brief Get all selected entities.
   */
  virtual std::vector<te::entity::EntityId> const& GetSelection() const = 0;
  
  /**
   * @brief Get the primary (first) selected entity.
   */
  virtual te::entity::EntityId GetPrimarySelection() const = 0;
  
  /**
   * @brief Get selection count.
   */
  virtual size_t GetSelectionCount() const = 0;
  
  /**
   * @brief Check if there is any selection.
   */
  virtual bool HasSelection() const = 0;
  
  // === Box Selection ===
  
  /**
   * @brief Start box selection.
   * @param startX Starting X coordinate
   * @param startY Starting Y coordinate
   */
  virtual void BeginBoxSelection(int startX, int startY) = 0;
  
  /**
   * @brief Update box selection.
   * @param currentX Current X coordinate
   * @param currentY Current Y coordinate
   */
  virtual void UpdateBoxSelection(int currentX, int currentY) = 0;
  
  /**
   * @brief End box selection and select entities within box.
   * @param addToExisting Add to current selection instead of replacing
   */
  virtual void EndBoxSelection(bool addToExisting) = 0;
  
  /**
   * @brief Check if box selection is active.
   */
  virtual bool IsBoxSelecting() const = 0;
  
  /**
   * @brief Get box selection rectangle.
   */
  virtual void GetBoxSelectionRect(int& x, int& y, int& width, int& height) const = 0;
  
  // === Selection Filter ===
  
  /**
   * @brief Set selection filter.
   */
  virtual void SetFilter(SelectionFilter const& filter) = 0;
  
  /**
   * @brief Get current selection filter.
   */
  virtual SelectionFilter GetFilter() const = 0;
  
  // === Events ===
  
  /**
   * @brief Set callback for selection changes.
   */
  virtual void SetOnSelectionChanged(std::function<void(SelectionChangeEvent const&)> callback) = 0;
  
  // === Selection Highlight ===
  
  /**
   * @brief Enable/disable selection highlight rendering.
   */
  virtual void SetHighlightEnabled(bool enabled) = 0;
  
  /**
   * @brief Check if selection highlight is enabled.
   */
  virtual bool IsHighlightEnabled() const = 0;
  
  /**
   * @brief Set highlight color.
   */
  virtual void SetHighlightColor(float r, float g, float b, float a) = 0;
  
  // === Selection Bounds ===
  
  /**
   * @brief Get the bounding box encompassing all selected entities.
   */
  virtual bool GetSelectionBounds(math::Vec3& min, math::Vec3& max) const = 0;
  
  /**
   * @brief Get the center point of selection bounds.
   */
  virtual bool GetSelectionCenter(math::Vec3& center) const = 0;
  
  // === Clipboard Operations ===
  
  /**
   * @brief Copy selected entities to clipboard.
   */
  virtual void CopySelection() = 0;
  
  /**
   * @brief Cut selected entities (copy + prepare for delete).
   */
  virtual void CutSelection() = 0;
  
  /**
   * @brief Check if clipboard has entities.
   */
  virtual bool HasClipboardData() const = 0;
  
  /**
   * @brief Select all entities in current level.
   */
  virtual void SelectAll() = 0;
  
  /**
   * @brief Invert selection.
   */
  virtual void InvertSelection() = 0;
};

/**
 * @brief Factory function to create a selection manager.
 */
ISelectionManager* CreateSelectionManager();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_SELECTION_MANAGER_H
