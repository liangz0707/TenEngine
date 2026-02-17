/**
 * @file MultiObjectEditing.cpp
 * @brief Multi-object editing implementation (024-Editor).
 */
#include <te/editor/MultiObjectEditing.h>
#include <te/core/log.h>
#include <imgui.h>
#include <algorithm>
#include <cmath>

namespace te {
namespace editor {

class MultiObjectEditorImpl : public IMultiObjectEditor {
public:
  MultiObjectEditorImpl()
    : m_pivotMode(MultiSelectionTransform::PivotMode::Center)
    , m_transformSpace(MultiSelectionTransform::TransformSpace::World)
    , m_activeEntity(te::entity::InvalidEntityId)
  {
  }

  // === Selection Access ===
  
  void SetSelection(std::vector<te::entity::EntityId> const& ids) override {
    m_selection = ids;
    
    if (!ids.empty()) {
      m_activeEntity = ids[0];
    } else {
      m_activeEntity = te::entity::InvalidEntityId;
    }
    
    UpdateSelectionCenter();
  }
  
  std::vector<te::entity::EntityId> const& GetSelection() const override {
    return m_selection;
  }
  
  size_t GetSelectionCount() const override {
    return m_selection.size();
  }
  
  bool HasSelection() const override {
    return !m_selection.empty();
  }
  
  // === Transform Operations ===
  
  MultiSelectionTransform GetTransform() const override {
    MultiSelectionTransform transform;
    transform.pivotMode = m_pivotMode;
    transform.transformSpace = m_transformSpace;
    
    if (m_selection.empty()) {
      return transform;
    }
    
    if (m_selection.size() == 1) {
      // Single object - no mixed values
      // TODO: Get actual transform from entity
      transform.position.value = math::Vec3(0, 0, 0);
      transform.rotation.value = math::Vec3(0, 0, 0);
      transform.scale.value = math::Vec3(1, 1, 1);
      transform.position.isSet = true;
      transform.rotation.isSet = true;
      transform.scale.isSet = true;
    } else {
      // Multiple objects - check for mixed values
      // For now, use center position
      transform.position.value = m_selectionCenter;
      transform.position.isMixed = true;
      transform.position.isSet = true;
    }
    
    return transform;
  }
  
  void SetPosition(math::Vec3 const& position, bool relative) override {
    if (m_selection.empty()) return;
    
    // TODO: Integrate with entity system
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Set position for selection");
    (void)position;
    (void)relative;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  void SetRotation(math::Vec3 const& rotation, bool relative) override {
    if (m_selection.empty()) return;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Set rotation for selection");
    (void)rotation;
    (void)relative;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  void SetScale(math::Vec3 const& scale, bool relative) override {
    if (m_selection.empty()) return;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Set scale for selection");
    (void)scale;
    (void)relative;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  void Move(math::Vec3 const& delta) override {
    if (m_selection.empty()) return;
    
    // TODO: Apply delta to all selected entities
    m_selectionCenter.x += delta.x;
    m_selectionCenter.y += delta.y;
    m_selectionCenter.z += delta.z;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Moved selection");
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  void Rotate(math::Vec3 const& delta) override {
    if (m_selection.empty()) return;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Rotated selection");
    (void)delta;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  void Scale(math::Vec3 const& factor) override {
    if (m_selection.empty()) return;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Scaled selection");
    (void)factor;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  // === Pivot Mode ===
  
  void SetPivotMode(MultiSelectionTransform::PivotMode mode) override {
    m_pivotMode = mode;
    UpdateSelectionCenter();
  }
  
  MultiSelectionTransform::PivotMode GetPivotMode() const override {
    return m_pivotMode;
  }
  
  void SetTransformSpace(MultiSelectionTransform::TransformSpace space) override {
    m_transformSpace = space;
  }
  
  MultiSelectionTransform::TransformSpace GetTransformSpace() const override {
    return m_transformSpace;
  }
  
  math::Vec3 GetSelectionCenter() const override {
    return m_selectionCenter;
  }
  
  void GetSelectionBounds(math::Vec3& min, math::Vec3& max) const override {
    // TODO: Calculate actual bounds from entity transforms
    min = m_selectionCenter - math::Vec3(1, 1, 1);
    max = m_selectionCenter + math::Vec3(1, 1, 1);
  }
  
  // === Property Editing ===
  
  std::vector<const char*> GetCommonProperties() const override {
    // TODO: Analyze common components and return shared properties
    return {"Position", "Rotation", "Scale", "Name"};
  }
  
  bool IsPropertyMixed(const char* propertyName) const override {
    // TODO: Compare property values across selection
    (void)propertyName;
    return m_selection.size() > 1;
  }
  
  bool SetProperty(const char* propertyName, void const* value) override {
    if (m_selection.empty() || !propertyName) return false;
    
    // TODO: Apply property to all selected entities
    te::core::Log(te::core::LogLevel::Info, 
                  ("MultiObjectEditor: Set property " + std::string(propertyName)).c_str());
    (void)value;
    
    return true;
  }
  
  bool ResetProperty(const char* propertyName) override {
    if (m_selection.empty() || !propertyName) return false;
    
    te::core::Log(te::core::LogLevel::Info, 
                  ("MultiObjectEditor: Reset property " + std::string(propertyName)).c_str());
    return true;
  }
  
  // === Component Operations ===
  
  std::vector<uint64_t> GetCommonComponents() const override {
    // TODO: Analyze components across selection
    return {};
  }
  
  bool AddComponent(uint64_t componentType) override {
    if (m_selection.empty()) return false;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Added component to selection");
    (void)componentType;
    return true;
  }
  
  bool RemoveComponent(uint64_t componentType) override {
    if (m_selection.empty()) return false;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Removed component from selection");
    (void)componentType;
    return true;
  }
  
  // === Group Operations ===
  
  te::entity::EntityId GroupSelected() override {
    if (m_selection.size() < 2) return te::entity::InvalidEntityId;
    
    // TODO: Create parent entity and reparent selected objects
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Grouped selection");
    
    // Return placeholder group ID
    return static_cast<te::entity::EntityId>(1000);
  }
  
  void UngroupSelected() override {
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Ungrouped selection");
  }
  
  // === Alignment ===
  
  void Align(int mode, int axis) override {
    if (m_selection.empty()) return;
    
    // 0 = Min, 1 = Center, 2 = Max
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Aligned selection");
    (void)mode;
    (void)axis;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  void Distribute(int axis) override {
    if (m_selection.size() < 3) return;
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Distributed selection");
    (void)axis;
    
    if (m_onTransformChanged) {
      m_onTransformChanged();
    }
  }
  
  // === Hierarchy ===
  
  bool ParentToActive() override {
    if (m_selection.empty() || 
        m_activeEntity == te::entity::InvalidEntityId) {
      return false;
    }
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Parented to active object");
    return true;
  }
  
  void UnparentSelected() override {
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Unparented selection");
  }
  
  // === Duplicate ===
  
  std::vector<te::entity::EntityId> DuplicateSelection() override {
    if (m_selection.empty()) return {};
    
    std::vector<te::entity::EntityId> newIds;
    
    // TODO: Integrate with entity system to create actual duplicates
    for (size_t i = 0; i < m_selection.size(); i++) {
      newIds.push_back(static_cast<te::entity::EntityId>(
        static_cast<uint64_t>(m_selection[i]) + 10000));
    }
    
    te::core::Log(te::core::LogLevel::Info, 
                  "MultiObjectEditor: Duplicated selection");
    
    return newIds;
  }
  
  std::vector<te::entity::EntityId> DuplicateAndMove(math::Vec3 const& offset) override {
    auto newIds = DuplicateSelection();
    
    if (!newIds.empty()) {
      // Store original selection, switch to new selection, move
      auto original = m_selection;
      m_selection = newIds;
      Move(offset);
      m_selection = original;
    }
    
    return newIds;
  }
  
  // === Deletion ===
  
  void DeleteSelection() override {
    if (m_selection.empty()) return;
    
    te::core::Log(te::core::LogLevel::Info, 
                  ("MultiObjectEditor: Deleted " + std::to_string(m_selection.size()) + 
                   " objects").c_str());
    
    m_selection.clear();
    m_activeEntity = te::entity::InvalidEntityId;
    m_selectionCenter = math::Vec3(0, 0, 0);
  }
  
  // === Active Object ===
  
  void SetActiveObject(te::entity::EntityId id) override {
    // Verify it's in selection
    for (auto const& eid : m_selection) {
      if (eid == id) {
        m_activeEntity = id;
        return;
      }
    }
  }
  
  te::entity::EntityId GetActiveObject() const override {
    return m_activeEntity;
  }
  
  // === Events ===
  
  void SetOnTransformChanged(std::function<void()> callback) override {
    m_onTransformChanged = std::move(callback);
  }

private:
  void UpdateSelectionCenter() {
    if (m_selection.empty()) {
      m_selectionCenter = math::Vec3(0, 0, 0);
      return;
    }
    
    if (m_pivotMode == MultiSelectionTransform::PivotMode::ActiveObject &&
        m_activeEntity != te::entity::InvalidEntityId) {
      // TODO: Get active object position
      m_selectionCenter = math::Vec3(0, 0, 0);
    } else if (m_pivotMode == MultiSelectionTransform::PivotMode::Center) {
      // Calculate center of all objects
      // TODO: Get actual positions from entity system
      m_selectionCenter = math::Vec3(0, 0, 0);
    } else {
      // Individual - each object its own pivot
      m_selectionCenter = math::Vec3(0, 0, 0);
    }
  }

private:
  std::vector<te::entity::EntityId> m_selection;
  math::Vec3 m_selectionCenter;
  
  MultiSelectionTransform::PivotMode m_pivotMode;
  MultiSelectionTransform::TransformSpace m_transformSpace;
  
  te::entity::EntityId m_activeEntity;
  
  std::function<void()> m_onTransformChanged;
};

IMultiObjectEditor* CreateMultiObjectEditor() {
  return new MultiObjectEditorImpl();
}

}  // namespace editor
}  // namespace te
