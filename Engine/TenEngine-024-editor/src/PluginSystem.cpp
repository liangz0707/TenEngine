/**
 * @file PluginSystem.cpp
 * @brief Plugin system implementation (024-Editor).
 */
#include <te/editor/PluginSystem.h>
#include <te/core/log.h>
#include <imgui.h>
#include <map>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <algorithm>

namespace te {
namespace editor {

class PluginSystemImpl : public IPluginSystem {
public:
  PluginSystemImpl()
    : m_nextHookId(1)
  {
  }

  // === Discovery ===

  size_t ScanForPlugins(char const* directory) override {
    if (!directory) return 0;

    // TODO: Actually scan directory for plugin files (.dll, .so, etc.)
    // For now, this is a placeholder that could be extended

    te::core::Log(te::core::LogLevel::Info,
                  ("PluginSystem: Scanning directory " + std::string(directory)).c_str());

    // Placeholder: scan for manifest files
    // In production, would look for plugin.json or similar

    return m_discoveredPlugins.size();
  }

  std::vector<PluginInfo const*> GetDiscoveredPlugins() const override {
    std::vector<PluginInfo const*> result;
    for (auto const& pair : m_discoveredPlugins) {
      result.push_back(&pair.second);
    }
    return result;
  }

  // === Loading ===

  bool LoadPlugin(char const* pluginId) override {
    if (!pluginId) return false;

    // Check if already loaded
    if (m_loadedPlugins.find(pluginId) != m_loadedPlugins.end()) {
      te::core::Log(te::core::LogLevel::Warn,
                    "PluginSystem: Plugin already loaded");
      return true;
    }

    // Find plugin info
    auto infoIt = m_discoveredPlugins.find(pluginId);
    if (infoIt == m_discoveredPlugins.end()) {
      // Plugin not discovered, try to load from path
      PluginInfo newInfo;
      newInfo.id = pluginId;
      m_discoveredPlugins[pluginId] = newInfo;
      infoIt = m_discoveredPlugins.find(pluginId);
    }

    PluginInfo& info = infoIt->second;

    // Check if enabled
    if (!info.enabled) {
      te::core::Log(te::core::LogLevel::Warn,
                    "PluginSystem: Plugin is disabled");
      return false;
    }

    // TODO: Actually load the plugin DLL/so
    // For now, create a placeholder plugin

    // In production, would:
    // 1. Load shared library
    // 2. Get plugin factory function
    // 3. Create plugin instance

    info.loaded = true;
    info.hasError = false;

    te::core::Log(te::core::LogLevel::Info,
                  ("PluginSystem: Loaded plugin " + std::string(pluginId)).c_str());

    // Trigger hook
    TriggerHook(PluginHook::OnEditorInit, nullptr);

    return true;
  }

  bool UnloadPlugin(char const* pluginId) override {
    if (!pluginId) return false;

    auto it = m_loadedPlugins.find(pluginId);
    if (it == m_loadedPlugins.end()) {
      return false;
    }

    // Call shutdown
    if (it->second) {
      it->second->Shutdown();
    }

    m_loadedPlugins.erase(it);

    // Update info
    auto infoIt = m_discoveredPlugins.find(pluginId);
    if (infoIt != m_discoveredPlugins.end()) {
      infoIt->second.loaded = false;
    }

    te::core::Log(te::core::LogLevel::Info,
                  ("PluginSystem: Unloaded plugin " + std::string(pluginId)).c_str());

    return true;
  }

  size_t LoadAllPlugins() override {
    size_t loaded = 0;

    std::vector<std::string> pluginIds;
    for (auto const& pair : m_discoveredPlugins) {
      pluginIds.push_back(pair.first);
    }

    for (auto const& id : pluginIds) {
      if (LoadPlugin(id.c_str())) {
        loaded++;
      }
    }

    return loaded;
  }

  void UnloadAllPlugins() override {
    std::vector<std::string> pluginIds;
    for (auto const& pair : m_loadedPlugins) {
      pluginIds.push_back(pair.first);
    }

    for (auto const& id : pluginIds) {
      UnloadPlugin(id.c_str());
    }
  }

  // === Query ===

  bool IsPluginLoaded(char const* pluginId) const override {
    return m_loadedPlugins.find(pluginId) != m_loadedPlugins.end();
  }

  PluginInfo const* GetPluginInfo(char const* pluginId) const override {
    auto it = m_discoveredPlugins.find(pluginId);
    if (it != m_discoveredPlugins.end()) {
      return &it->second;
    }
    return nullptr;
  }

  IPlugin* GetPlugin(char const* pluginId) override {
    auto it = m_loadedPlugins.find(pluginId);
    if (it != m_loadedPlugins.end()) {
      return it->second;
    }
    return nullptr;
  }

  std::vector<IPlugin*> GetLoadedPlugins() override {
    std::vector<IPlugin*> result;
    for (auto const& pair : m_loadedPlugins) {
      if (pair.second) {
        result.push_back(pair.second);
      }
    }
    return result;
  }

  // === Enable/Disable ===

  void SetPluginEnabled(char const* pluginId, bool enabled) override {
    auto it = m_discoveredPlugins.find(pluginId);
    if (it != m_discoveredPlugins.end()) {
      it->second.enabled = enabled;
    }
  }

  bool IsPluginEnabled(char const* pluginId) const override {
    auto it = m_discoveredPlugins.find(pluginId);
    if (it != m_discoveredPlugins.end()) {
      return it->second.enabled;
    }
    return false;
  }

  // === Hooks ===

  int RegisterHook(PluginHook hook, std::function<void(void*)> callback) override {
    int id = m_nextHookId++;
    m_hooks[hook].push_back({id, callback});
    return id;
  }

  void UnregisterHook(PluginHook hook, int callbackId) override {
    auto it = m_hooks.find(hook);
    if (it != m_hooks.end()) {
      auto& callbacks = it->second;
      callbacks.erase(
        std::remove_if(callbacks.begin(), callbacks.end(),
                       [callbackId](HookCallback const& hc) { return hc.id == callbackId; }),
        callbacks.end());
    }
  }

  void TriggerHook(PluginHook hook, void* userData) override {
    auto it = m_hooks.find(hook);
    if (it != m_hooks.end()) {
      for (auto const& hc : it->second) {
        if (hc.callback) {
          hc.callback(userData);
        }
      }
    }

    // Also notify loaded plugins
    for (auto const& pair : m_loadedPlugins) {
      if (pair.second) {
        pair.second->OnHook(hook, userData);
      }
    }
  }

  // === Menu Items ===

  std::vector<PluginMenuItem> GetAllPluginMenuItems() override {
    std::vector<PluginMenuItem> result;

    for (auto const& pair : m_loadedPlugins) {
      if (pair.second) {
        auto items = pair.second->GetMenuItems();
        for (auto& item : items) {
          result.push_back(std::move(item));
        }
      }
    }

    return result;
  }

  // === Panels ===

  std::vector<PluginPanel> GetAllPluginPanels() override {
    std::vector<PluginPanel> result;

    for (auto const& pair : m_loadedPlugins) {
      if (pair.second) {
        auto panels = pair.second->GetPanels();
        for (auto& panel : panels) {
          result.push_back(std::move(panel));
        }
      }
    }

    return result;
  }

  // === Settings ===

  bool SavePluginSettings(char const* pluginId) override {
    if (!pluginId) return false;

    // TODO: Serialize plugin settings to JSON

    return true;
  }

  bool LoadPluginSettings(char const* pluginId) override {
    if (!pluginId) return false;

    // TODO: Load plugin settings from JSON

    return true;
  }

  // === Hot Reload ===

  bool ReloadPlugin(char const* pluginId) override {
    if (!pluginId) return false;

    // Save state
    bool wasEnabled = IsPluginEnabled(pluginId);

    // Unload
    if (IsPluginLoaded(pluginId)) {
      UnloadPlugin(pluginId);
    }

    // Reload
    if (LoadPlugin(pluginId)) {
      SetPluginEnabled(pluginId, wasEnabled);
      return true;
    }

    return false;
  }

private:
  struct HookCallback {
    int id;
    std::function<void(void*)> callback;
  };

  int m_nextHookId;

  std::map<std::string, PluginInfo> m_discoveredPlugins;
  std::map<std::string, IPlugin*> m_loadedPlugins;
  std::map<PluginHook, std::vector<HookCallback>> m_hooks;
};

IPluginSystem* CreatePluginSystem() {
  return new PluginSystemImpl();
}

}  // namespace editor
}  // namespace te
