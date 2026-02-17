/**
 * @file LayoutManager.cpp
 * @brief Dock layout manager implementation (024-Editor).
 */
#include <te/editor/LayoutManager.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstring>

namespace te {
namespace editor {

class LayoutManagerImpl : public ILayoutManager {
public:
  LayoutManagerImpl()
    : m_currentLayout("Default")
    , m_viewportLayout(ViewportLayout::Single)
  {
    InitializeBuiltinLayouts();
    ApplyLayout("Default");
  }

  // === Current Layout ===
  
  bool ApplyLayout(const char* name) override {
    if (!name) return false;
    
    LayoutDef const* layout = GetLayout(name);
    if (!layout) return false;
    
    m_currentLayout = name;
    m_viewportLayout = layout->viewportLayout;
    
    // Apply panel visibility
    for (auto const& panel : layout->panels) {
      SetPanelVisible(panel.panelId, panel.visible);
      if (panel.docked) {
        DockPanel(panel.panelId);
      } else {
        FloatPanel(panel.panelId);
      }
      SetPanelDockRect(panel.panelId, panel.dockRect);
    }
    
    NotifyLayoutChanged(name);
    return true;
  }
  
  const char* GetCurrentLayoutName() const override {
    return m_currentLayout.c_str();
  }
  
  void SaveCurrentLayout() override {
    SaveCurrentLayoutAs(m_currentLayout.c_str());
  }
  
  void SaveCurrentLayoutAs(const char* name) override {
    if (!name) return;
    
    // Find existing layout or create new
    LayoutDef* layout = nullptr;
    for (auto& l : m_layouts) {
      if (l.name && std::string(l.name) == name) {
        layout = &l;
        break;
      }
    }
    
    if (!layout) {
      LayoutDef newLayout;
      newLayout.name = strdup(name);
      newLayout.isBuiltin = false;
      m_layouts.push_back(newLayout);
      layout = &m_layouts.back();
    }
    
    if (!layout->isBuiltin) {
      layout->viewportLayout = m_viewportLayout;
      layout->panels = m_panelStates;
      m_currentLayout = name;
    }
  }
  
  // === Layout Management ===
  
  void AddLayout(LayoutDef const& layout) override {
    if (!layout.name) return;
    
    // Remove existing layout with same name
    RemoveLayout(layout.name);
    
    LayoutDef copy = layout;
    copy.isBuiltin = false;
    m_layouts.push_back(copy);
  }
  
  bool RemoveLayout(const char* name) override {
    if (!name) return false;
    
    for (auto it = m_layouts.begin(); it != m_layouts.end(); ++it) {
      if (it->name && std::string(it->name) == name && !it->isBuiltin) {
        m_layouts.erase(it);
        return true;
      }
    }
    return false;
  }
  
  std::vector<LayoutDef> const& GetLayouts() const override {
    return m_layouts;
  }
  
  LayoutDef const* GetLayout(const char* name) const override {
    if (!name) return nullptr;
    
    for (auto const& layout : m_layouts) {
      if (layout.name && std::string(layout.name) == name) {
        return &layout;
      }
    }
    return nullptr;
  }
  
  bool HasLayout(const char* name) const override {
    return GetLayout(name) != nullptr;
  }
  
  // === Panel Management ===
  
  void SetPanelVisible(const char* panelId, bool visible) override {
    if (!panelId) return;
    
    PanelVisibility* panel = GetOrCreatePanel(panelId);
    if (panel && panel->visible != visible) {
      panel->visible = visible;
      NotifyPanelVisibilityChanged(panelId, visible);
    }
  }
  
  bool IsPanelVisible(const char* panelId) const override {
    if (!panelId) return false;
    
    for (auto const& panel : m_panelStates) {
      if (panel.panelId && std::string(panel.panelId) == panelId) {
        return panel.visible;
      }
    }
    return true;  // Default visible
  }
  
  void TogglePanel(const char* panelId) override {
    SetPanelVisible(panelId, !IsPanelVisible(panelId));
  }
  
  void DockPanel(const char* panelId) override {
    if (!panelId) return;
    
    PanelVisibility* panel = GetOrCreatePanel(panelId);
    if (panel) {
      panel->docked = true;
    }
  }
  
  void FloatPanel(const char* panelId) override {
    if (!panelId) return;
    
    PanelVisibility* panel = GetOrCreatePanel(panelId);
    if (panel) {
      panel->docked = false;
    }
  }
  
  bool IsPanelDocked(const char* panelId) const override {
    if (!panelId) return true;
    
    for (auto const& panel : m_panelStates) {
      if (panel.panelId && std::string(panel.panelId) == panelId) {
        return panel.docked;
      }
    }
    return true;  // Default docked
  }
  
  void SetPanelDockRect(const char* panelId, EditorRect const& rect) override {
    if (!panelId) return;
    
    PanelVisibility* panel = GetOrCreatePanel(panelId);
    if (panel) {
      panel->dockRect = rect;
    }
  }
  
  std::vector<PanelVisibility> const& GetPanelStates() const override {
    return m_panelStates;
  }
  
  // === Viewport Layout ===
  
  void SetViewportLayout(ViewportLayout layout) override {
    m_viewportLayout = layout;
  }
  
  ViewportLayout GetViewportLayout() const override {
    return m_viewportLayout;
  }
  
  // === Persistence ===
  
  bool SaveToFile(const char* path) override {
    if (!path) return false;
    
    std::ofstream file(path);
    if (!file.is_open()) return false;
    
    // Save current layout name
    file << "current=" << m_currentLayout << "\n";
    file << "viewport=" << static_cast<int>(m_viewportLayout) << "\n";
    
    // Save panel states
    file << "[Panels]\n";
    for (auto const& panel : m_panelStates) {
      if (panel.panelId) {
        file << panel.panelId << "=" 
             << (panel.visible ? 1 : 0) << ","
             << (panel.docked ? 1 : 0) << ","
             << panel.dockRect.x << ","
             << panel.dockRect.y << ","
             << panel.dockRect.width << ","
             << panel.dockRect.height << "\n";
      }
    }
    
    // Save custom layouts
    file << "[Layouts]\n";
    for (auto const& layout : m_layouts) {
      if (!layout.isBuiltin && layout.name) {
        file << "layout=" << layout.name << "\n";
        file << "viewport=" << static_cast<int>(layout.viewportLayout) << "\n";
        for (auto const& panel : layout.panels) {
          if (panel.panelId) {
            file << "panel=" << panel.panelId << "," 
                 << (panel.visible ? 1 : 0) << "\n";
          }
        }
        file << "endlayout\n";
      }
    }
    
    return true;
  }
  
  bool LoadFromFile(const char* path) override {
    if (!path) return false;
    
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    std::string line;
    std::string section;
    std::string currentLayoutName;
    LayoutDef* currentLayoutDef = nullptr;
    
    while (std::getline(file, line)) {
      if (line.empty() || line[0] == '#') continue;
      
      // Check for section
      if (line[0] == '[') {
        section = line.substr(1, line.find(']') - 1);
        continue;
      }
      
      size_t eqPos = line.find('=');
      if (eqPos == std::string::npos) continue;
      
      std::string key = line.substr(0, eqPos);
      std::string value = line.substr(eqPos + 1);
      
      if (section.empty()) {
        if (key == "current") {
          m_currentLayout = value;
        } else if (key == "viewport") {
          m_viewportLayout = static_cast<ViewportLayout>(std::stoi(value));
        }
      } else if (section == "Panels") {
        ParsePanelState(key, value);
      } else if (section == "Layouts") {
        if (key == "layout") {
          currentLayoutName = value;
          // Add new layout
          LayoutDef newLayout;
          newLayout.name = strdup(value.c_str());
          newLayout.isBuiltin = false;
          m_layouts.push_back(newLayout);
          currentLayoutDef = &m_layouts.back();
        } else if (key == "viewport" && currentLayoutDef) {
          currentLayoutDef->viewportLayout = static_cast<ViewportLayout>(std::stoi(value));
        } else if (key == "panel" && currentLayoutDef) {
          PanelVisibility pv;
          size_t comma = value.find(',');
          if (comma != std::string::npos) {
            pv.panelId = strdup(value.substr(0, comma).c_str());
            pv.visible = std::stoi(value.substr(comma + 1)) != 0;
          }
          currentLayoutDef->panels.push_back(pv);
        } else if (key == "endlayout") {
          currentLayoutDef = nullptr;
        }
      }
    }
    
    return true;
  }
  
  void ResetToDefault() override {
    ApplyLayout("Default");
  }
  
  // === Events ===
  
  void SetOnLayoutChanged(std::function<void(const char*)> callback) override {
    m_onLayoutChanged = std::move(callback);
  }
  
  void SetOnPanelVisibilityChanged(
      std::function<void(const char*, bool)> callback) override {
    m_onPanelVisibilityChanged = std::move(callback);
  }

private:
  void InitializeBuiltinLayouts() {
    // Default layout
    {
      LayoutDef layout;
      layout.name = "Default";
      layout.viewportLayout = ViewportLayout::Single;
      layout.isBuiltin = true;
      layout.panels = {
        {"SceneView", true, true, {0, 0, 250, 400}},
        {"Properties", true, true, {0, 400, 250, 300}},
        {"Resources", true, true, {0, 700, 250, 200}},
        {"Console", true, true, {0, 0, 0, 200}},
        {"Profiler", false, true, {0, 0, 0, 0}},
        {"Statistics", false, true, {0, 0, 0, 0}}
      };
      m_layouts.push_back(layout);
    }
    
    // 2D layout
    {
      LayoutDef layout;
      layout.name = "2D";
      layout.viewportLayout = ViewportLayout::Single;
      layout.isBuiltin = true;
      layout.panels = {
        {"SceneView", true, true, {0, 0, 200, 600}},
        {"Properties", true, true, {0, 0, 300, 400}},
        {"Console", true, true, {0, 0, 0, 150}},
        {"Resources", false, true, {0, 0, 0, 0}},
        {"Profiler", false, true, {0, 0, 0, 0}},
        {"Statistics", false, true, {0, 0, 0, 0}}
      };
      m_layouts.push_back(layout);
    }
    
    // 3D layout
    {
      LayoutDef layout;
      layout.name = "3D";
      layout.viewportLayout = ViewportLayout::Quad;
      layout.isBuiltin = true;
      layout.panels = {
        {"SceneView", true, true, {0, 0, 300, 500}},
        {"Properties", true, true, {0, 500, 300, 300}},
        {"Console", true, true, {0, 0, 0, 200}},
        {"Resources", true, true, {0, 0, 200, 400}},
        {"Profiler", false, true, {0, 0, 0, 0}},
        {"Statistics", false, true, {0, 0, 0, 0}}
      };
      m_layouts.push_back(layout);
    }
    
    // Wide layout
    {
      LayoutDef layout;
      layout.name = "Wide";
      layout.viewportLayout = ViewportLayout::TwoHorizontal;
      layout.isBuiltin = true;
      layout.panels = {
        {"SceneView", true, true, {0, 0, 250, 500}},
        {"Properties", true, true, {0, 500, 250, 300}},
        {"Console", true, true, {0, 0, 0, 200}},
        {"Resources", true, true, {0, 0, 250, 300}},
        {"Profiler", true, true, {0, 0, 250, 200}},
        {"Statistics", false, true, {0, 0, 0, 0}}
      };
      m_layouts.push_back(layout);
    }
  }
  
  PanelVisibility* GetOrCreatePanel(const char* panelId) {
    for (auto& panel : m_panelStates) {
      if (panel.panelId && std::string(panel.panelId) == panelId) {
        return &panel;
      }
    }
    
    // Create new panel state
    PanelVisibility panel;
    panel.panelId = strdup(panelId);
    panel.visible = true;
    panel.docked = true;
    m_panelStates.push_back(panel);
    return &m_panelStates.back();
  }
  
  void ParsePanelState(std::string const& panelId, std::string const& value) {
    PanelVisibility panel;
    panel.panelId = strdup(panelId.c_str());
    
    // Parse: visible,docked,x,y,w,h
    size_t pos = 0;
    size_t next = value.find(',', pos);
    
    if (next != std::string::npos) {
      panel.visible = std::stoi(value.substr(pos, next - pos)) != 0;
      pos = next + 1;
      
      next = value.find(',', pos);
      if (next != std::string::npos) {
        panel.docked = std::stoi(value.substr(pos, next - pos)) != 0;
        pos = next + 1;
        
        next = value.find(',', pos);
        if (next != std::string::npos) {
          panel.dockRect.x = std::stoi(value.substr(pos, next - pos));
          pos = next + 1;
          
          next = value.find(',', pos);
          if (next != std::string::npos) {
            panel.dockRect.y = std::stoi(value.substr(pos, next - pos));
            pos = next + 1;
            
            next = value.find(',', pos);
            if (next != std::string::npos) {
              panel.dockRect.width = std::stoi(value.substr(pos, next - pos));
              pos = next + 1;
              panel.dockRect.height = std::stoi(value.substr(pos));
            }
          }
        }
      }
    }
    
    // Update or add
    bool found = false;
    for (auto& p : m_panelStates) {
      if (p.panelId && std::string(p.panelId) == panelId) {
        p = panel;
        found = true;
        break;
      }
    }
    
    if (!found) {
      m_panelStates.push_back(panel);
    }
  }
  
  void NotifyLayoutChanged(const char* name) {
    if (m_onLayoutChanged) {
      m_onLayoutChanged(name);
    }
  }
  
  void NotifyPanelVisibilityChanged(const char* panelId, bool visible) {
    if (m_onPanelVisibilityChanged) {
      m_onPanelVisibilityChanged(panelId, visible);
    }
  }
  
  // Layout storage
  std::vector<LayoutDef> m_layouts;
  std::string m_currentLayout;
  
  // Panel states
  std::vector<PanelVisibility> m_panelStates;
  
  // Viewport layout
  ViewportLayout m_viewportLayout;
  
  // Callbacks
  std::function<void(const char*)> m_onLayoutChanged;
  std::function<void(const char*, bool)> m_onPanelVisibilityChanged;
};

ILayoutManager* CreateLayoutManager() {
  return new LayoutManagerImpl();
}

}  // namespace editor
}  // namespace te
