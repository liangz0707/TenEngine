/**
 * @file MultiObjectEditing.h
 * @brief Multi-object editing interface.
 */
#ifndef TE_EDITOR_MULTI_OBJECT_EDITING_H
#define TE_EDITOR_MULTI_OBJECT_EDITING_H

#include <te/editor/EditorTypes.h>
#include <te/entity/EntityId.h>
#include <te/core/math.h>
#include <functional>
#include <vector>
#include <cstdint>

namespace te {
namespace editor {

// Math type aliases for convenience
namespace math {
using Vec3 = te::core::Vector3;
}

/**
 * @brief Property value with mixed-state support.
 */
template<typename T>
struct MixedValue {
  T value = T{};
  bool isMixed = false;    ///< True if different values across selection
  bool isSet = false;      ///< True if value is set (vs default)
};

// Note: MultiSelectionTransform is defined in EditorTypes.h

/**
 * @brief Multi-object editor interface.
 *
 * Provides editing operations that work across multiple selected objects.
 */
class IMultiObjectEditor {
public:
  virtual ~IMultiObjectEditor() = default;

  // === Selection Access ===

  /**
   * @brief Set the objects to edit.
   */
  virtual void SetSelection(std::vector<te::entity::EntityId> const& ids) = 0;

  /**
   * @brief Get current selection.
   */
  virtual std::vector<te::entity::EntityId> const& GetSelection() const = 0;

  /**
   * @brief Get selection count.
   */
  virtual size_t GetSelectionCount() const = 0;

  /**
   * @brief Check if has selection.
   */
  virtual bool HasSelection() const = 0;

  // === Transform Operations ===

  /**
   * @brief Get combined transform of selection.
   */
  virtual MultiSelectionTransform GetTransform() const = 0;

  /**
   * @brief Set position for all selected objects.
   * @param position New position
   * @param relative If true, apply as offset
   */
  virtual void SetPosition(math::Vec3 const& position, bool relative = false) = 0;

  /**
   * @brief Set rotation for all selected objects.
   */
  virtual void SetRotation(math::Vec3 const& rotation, bool relative = false) = 0;

  /**
   * @brief Set scale for all selected objects.
   */
  virtual void SetScale(math::Vec3 const& scale, bool relative = false) = 0;

  /**
   * @brief Move objects by delta.
   */
  virtual void Move(math::Vec3 const& delta) = 0;

  /**
   * @brief Rotate objects by delta.
   */
  virtual void Rotate(math::Vec3 const& delta) = 0;

  /**
   * @brief Scale objects by factor.
   */
  virtual void Scale(math::Vec3 const& factor) = 0;

  // === Pivot Mode ===

  /**
   * @brief Set pivot mode.
   */
  virtual void SetPivotMode(PivotMode mode) = 0;

  /**
   * @brief Get pivot mode.
   */
  virtual PivotMode GetPivotMode() const = 0;

  /**
   * @brief Set transform space.
   */
  virtual void SetTransformSpace(CoordinateSpace space) = 0;

  /**
   * @brief Get transform space.
   */
  virtual CoordinateSpace GetTransformSpace() const = 0;

  /**
   * @brief Get selection center point.
   */
  virtual math::Vec3 GetSelectionCenter() const = 0;

  /**
   * @brief Get selection bounds.
   */
  virtual void GetSelectionBounds(math::Vec3& min, math::Vec3& max) const = 0;

  // === Property Editing ===

  /**
   * @brief Get common properties across selection.
   */
  virtual std::vector<const char*> GetCommonProperties() const = 0;

  /**
   * @brief Check if property has mixed values.
   */
  virtual bool IsPropertyMixed(const char* propertyName) const = 0;

  /**
   * @brief Set property value for all selected objects.
   */
  virtual bool SetProperty(const char* propertyName, void const* value) = 0;

  /**
   * @brief Reset property to default for all selected objects.
   */
  virtual bool ResetProperty(const char* propertyName) = 0;

  // === Component Operations ===

  /**
   * @brief Get common component types.
   */
  virtual std::vector<uint64_t> GetCommonComponents() const = 0;

  /**
   * @brief Add component to all selected objects.
   */
  virtual bool AddComponent(uint64_t componentType) = 0;

  /**
   * @brief Remove component from all selected objects.
   */
  virtual bool RemoveComponent(uint64_t componentType) = 0;

  // === Group Operations ===

  /**
   * @brief Group selected objects.
   * @return New group entity ID
   */
  virtual te::entity::EntityId GroupSelected() = 0;

  /**
   * @brief Ungroup selected objects.
   */
  virtual void UngroupSelected() = 0;

  // === Alignment ===

  /**
   * @brief Align selection.
   * @param mode Alignment mode (Min/Center/Max)
   * @param axis Axis to align (0=X, 1=Y, 2=Z)
   */
  virtual void Align(int mode, int axis) = 0;

  /**
   * @brief Distribute selection evenly.
   * @param axis Axis to distribute (0=X, 1=Y, 2=Z)
   */
  virtual void Distribute(int axis) = 0;

  // === Hierarchy ===

  /**
   * @brief Parent selected objects to active object.
   */
  virtual bool ParentToActive() = 0;

  /**
   * @brief Unparent selected objects.
   */
  virtual void UnparentSelected() = 0;

  // === Duplicate ===

  /**
   * @brief Duplicate selected objects.
   * @return New entity IDs
   */
  virtual std::vector<te::entity::EntityId> DuplicateSelection() = 0;

  /**
   * @brief Duplicate and move.
   */
  virtual std::vector<te::entity::EntityId> DuplicateAndMove(math::Vec3 const& offset) = 0;

  // === Deletion ===

  /**
   * @brief Delete selected objects.
   */
  virtual void DeleteSelection() = 0;

  // === Active Object ===

  /**
   * @brief Set active object in selection.
   */
  virtual void SetActiveObject(te::entity::EntityId id) = 0;

  /**
   * @brief Get active object.
   */
  virtual te::entity::EntityId GetActiveObject() const = 0;

  // === Events ===

  /**
   * @brief Set callback for selection transform change.
   */
  virtual void SetOnTransformChanged(std::function<void()> callback) = 0;
};

/**
 * @brief Factory function to create multi-object editor.
 */
IMultiObjectEditor* CreateMultiObjectEditor();

}  // namespace editor
}  // namespace te

#endif  // TE_EDITOR_MULTI_OBJECT_EDITING_H
