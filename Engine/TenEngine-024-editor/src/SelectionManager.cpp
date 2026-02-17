/**
 * @file SelectionManager.cpp
 * @brief Selection manager implementation (024-Editor).
 */
#include <te/editor/SelectionManager.h>
#include <te/entity/Entity.h>
#include <te/entity/EntityManager.h>
#include <te/scene/ISceneNode.h>
#include <algorithm>
#include <cmath>

namespace te {
namespace editor {

class SelectionManagerImpl : public ISelectionManager {
public:
  SelectionManagerImpl()
    : m_highlightEnabled(true)
    , m_highlightR(1.0f)
    , m_highlightG(0.6f)
    , m_highlightB(0.0f)
    , m_highlightA(0.5f)
    , m_boxSelecting(false)
    , m_boxStartX(0)
    , m_boxStartY(0)
    , m_boxCurrentX(0)
    , m_boxCurrentY(0)
  {
    m_filter.selectEntities = true;
    m_filter.selectResources = false;
    m_filter.selectComponents = false;
  }

  // === Selection Operations ===
  
  void Select(te::entity::EntityId id) override {
    std::vector<te::entity::EntityId> prev = m_selection;
    m_selection.clear();
    m_selection.push_back(id);
    FireSelectionChange(prev, false);
  }
  
  void SelectMultiple(std::vector<te::entity::EntityId> const& ids) override {
    std::vector<te::entity::EntityId> prev = m_selection;
    m_selection = ids;
    FireSelectionChange(prev, false);
  }
  
  void AddToSelection(te::entity::EntityId id) override {
    if (!IsSelected(id)) {
      std::vector<te::entity::EntityId> prev = m_selection;
      m_selection.push_back(id);
      FireSelectionChange(prev, true);
    }
  }
  
  void ToggleSelection(te::entity::EntityId id) override {
    std::vector<te::entity::EntityId> prev = m_selection;
    
    auto it = std::find_if(m_selection.begin(), m_selection.end(),
      [&id](te::entity::EntityId const& e) { return e.value == id.value; });
    
    if (it != m_selection.end()) {
      m_selection.erase(it);
    } else {
      m_selection.push_back(id);
    }
    
    FireSelectionChange(prev, true);
  }
  
  void Deselect(te::entity::EntityId id) override {
    auto it = std::find_if(m_selection.begin(), m_selection.end(),
      [&id](te::entity::EntityId const& e) { return e.value == id.value; });
    
    if (it != m_selection.end()) {
      std::vector<te::entity::EntityId> prev = m_selection;
      m_selection.erase(it);
      FireSelectionChange(prev, false);
    }
  }
  
  void ClearSelection() override {
    if (!m_selection.empty()) {
      std::vector<te::entity::EntityId> prev = m_selection;
      m_selection.clear();
      FireSelectionChange(prev, false);
    }
  }
  
  // === Selection Query ===
  
  bool IsSelected(te::entity::EntityId id) const override {
    for (auto const& e : m_selection) {
      if (e.value == id.value) return true;
    }
    return false;
  }
  
  std::vector<te::entity::EntityId> const& GetSelection() const override {
    return m_selection;
  }
  
  te::entity::EntityId GetPrimarySelection() const override {
    if (m_selection.empty()) {
      return te::entity::EntityId{0};
    }
    return m_selection[0];
  }
  
  size_t GetSelectionCount() const override {
    return m_selection.size();
  }
  
  bool HasSelection() const override {
    return !m_selection.empty();
  }
  
  // === Box Selection ===
  
  void BeginBoxSelection(int startX, int startY) override {
    m_boxSelecting = true;
    m_boxStartX = startX;
    m_boxStartY = startY;
    m_boxCurrentX = startX;
    m_boxCurrentY = startY;
  }
  
  void UpdateBoxSelection(int currentX, int currentY) override {
    if (m_boxSelecting) {
      m_boxCurrentX = currentX;
      m_boxCurrentY = currentY;
    }
  }
  
  void EndBoxSelection(bool addToExisting) override {
    if (!m_boxSelecting) return;
    m_boxSelecting = false;
    
    // Calculate box rect
    int x = std::min(m_boxStartX, m_boxCurrentX);
    int y = std::min(m_boxStartY, m_boxCurrentY);
    int w = std::abs(m_boxCurrentX - m_boxStartX);
    int h = std::abs(m_boxCurrentY - m_boxStartY);
    
    if (w < 2 && h < 2) {
      // Too small, treat as click
      return;
    }
    
    // Box selection requires viewport integration
    // This is a placeholder - actual implementation would:
    // 1. Frustum cull entities within the box
    // 2. Apply selection based on addToExisting flag
    
    if (!addToExisting) {
      std::vector<te::entity::EntityId> prev = m_selection;
      m_selection.clear();
      // TODO: Add entities within box to m_selection
      FireSelectionChange(prev, addToExisting);
    }
  }
  
  bool IsBoxSelecting() const override {
    return m_boxSelecting;
  }
  
  void GetBoxSelectionRect(int& x, int& y, int& width, int& height) const override {
    x = std::min(m_boxStartX, m_boxCurrentX);
    y = std::min(m_boxStartY, m_boxCurrentY);
    width = std::abs(m_boxCurrentX - m_boxStartX);
    height = std::abs(m_boxCurrentY - m_boxStartY);
  }
  
  // === Selection Filter ===
  
  void SetFilter(SelectionFilter const& filter) override {
    m_filter = filter;
  }
  
  SelectionFilter GetFilter() const override {
    return m_filter;
  }
  
  // === Events ===
  
  void SetOnSelectionChanged(std::function<void(SelectionChangeEvent const&)> callback) override {
    m_onSelectionChanged = std::move(callback);
  }
  
  // === Selection Highlight ===
  
  void SetHighlightEnabled(bool enabled) override {
    m_highlightEnabled = enabled;
  }
  
  bool IsHighlightEnabled() const override {
    return m_highlightEnabled;
  }
  
  void SetHighlightColor(float r, float g, float b, float a) override {
    m_highlightR = r;
    m_highlightG = g;
    m_highlightB = b;
    m_highlightA = a;
  }
  
  // === Selection Bounds ===
  
  bool GetSelectionBounds(te::math::Vec3& min, te::math::Vec3& max) const override {
    if (m_selection.empty()) return false;
    
    min = te::math::Vec3(1e10f, 1e10f, 1e10f);
    max = te::math::Vec3(-1e10f, -1e10f, -1e10f);
    
    for (auto const& id : m_selection) {
      te::entity::Entity* entity = te::entity::EntityManager::GetInstance().GetEntity(id);
      if (!entity) continue;
      
      te::scene::Transform t = entity->GetLocalTransform();
      
      // Expand bounds to include entity position
      // TODO: Get actual entity bounds when mesh info is available
      min.x = std::min(min.x, t.position.x);
      min.y = std::min(min.y, t.position.y);
      min.z = std::min(min.z, t.position.z);
      max.x = std::max(max.x, t.position.x);
      max.y = std::max(max.y, t.position.y);
      max.z = std::max(max.z, t.position.z);
    }
    
    return true;
  }
  
  bool GetSelectionCenter(te::math::Vec3& center) const override {
    te::math::Vec3 min, max;
    if (!GetSelectionBounds(min, max)) return false;
    
    center = (min + max) * 0.5f;
    return true;
  }
  
  // === Clipboard Operations ===
  
  void CopySelection() override {
    m_clipboard.clear();
    
    for (auto const& id : m_selection) {
      te::entity::Entity* entity = te::entity::EntityManager::GetInstance().GetEntity(id);
      if (!entity) continue;
      
      // Store entity info for later paste
      ClipboardEntry entry;
      entry.name = entity->GetName() ? entity->GetName() : "";
      entry.transform = entity->GetLocalTransform();
      // TODO: Copy component data
      m_clipboard.push_back(entry);
    }
  }
  
  void CutSelection() override {
    CopySelection();
    // Mark for deletion on paste - actual deletion happens after paste
    m_cutMode = true;
  }
  
  bool HasClipboardData() const override {
    return !m_clipboard.empty();
  }
  
  void SelectAll() override {
    // TODO: Requires iteration over all entities in current level
    // Placeholder - would need WorldManager/EntityManager integration
  }
  
  void InvertSelection() override {
    // TODO: Requires knowing all selectable entities
    // Placeholder - would need WorldManager/EntityManager integration
  }

private:
  void FireSelectionChange(std::vector<te::entity::EntityId> const& prev, bool additive) {
    if (m_onSelectionChanged) {
      SelectionChangeEvent evt;
      evt.previousSelection = prev;
      evt.currentSelection = m_selection;
      evt.isAdditive = additive;
      m_onSelectionChanged(evt);
    }
  }
  
  // Selection state
  std::vector<te::entity::EntityId> m_selection;
  SelectionFilter m_filter;
  
  // Highlight
  bool m_highlightEnabled;
  float m_highlightR, m_highlightG, m_highlightB, m_highlightA;
  
  // Box selection
  bool m_boxSelecting;
  int m_boxStartX, m_boxStartY;
  int m_boxCurrentX, m_boxCurrentY;
  
  // Clipboard
  struct ClipboardEntry {
    std::string name;
    te::scene::Transform transform;
    // TODO: Component data
  };
  std::vector<ClipboardEntry> m_clipboard;
  bool m_cutMode = false;
  
  // Callbacks
  std::function<void(SelectionChangeEvent const&)> m_onSelectionChanged;
};

ISelectionManager* CreateSelectionManager() {
  return new SelectionManagerImpl();
}

}  // namespace editor
}  // namespace te
