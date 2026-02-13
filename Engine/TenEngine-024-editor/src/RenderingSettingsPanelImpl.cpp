/**
 * @file RenderingSettingsPanelImpl.cpp
 * @brief Rendering settings panel (024-Editor).
 */
#include <te/editor/RenderingSettingsPanel.h>
#include <te/core/platform.h>
#include <sstream>
#include <cstring>

namespace te {
namespace editor {

namespace {
std::string EscapeJsonString(std::string const& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else if (c == '\t') out += "\\t";
    else out += c;
  }
  return out;
}

std::string ConfigToJson(RenderingConfig const& c) {
  std::ostringstream oss;
  oss << "{\"renderPath\":\"" << EscapeJsonString(c.renderPath)
      << "\",\"enableShadows\":" << (c.enableShadows ? "true" : "false")
      << ",\"enableIBL\":" << (c.enableIBL ? "true" : "false")
      << ",\"exposure\":" << c.exposure << "}";
  return oss.str();
}

bool JsonToConfig(char const* json, RenderingConfig& out) {
  if (!json) return false;
  char const* p = json;
  auto skip = [&]() { while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p; };
  auto match = [&](char c) { skip(); if (*p == c) { ++p; return true; } return false; };
  auto readStr = [&]() -> std::string {
    skip();
    if (*p != '"') return "";
    ++p;
    std::string s;
    while (*p && *p != '"') {
      if (*p == '\\') { ++p; if (*p) s += *p++; }
      else s += *p++;
    }
    if (*p == '"') ++p;
    return s;
  };
  auto readBool = [&]() -> bool {
    skip();
    if (std::strncmp(p, "true", 4) == 0) { p += 4; return true; }
    if (std::strncmp(p, "false", 5) == 0) { p += 5; return false; }
    return false;
  };
  auto readFloat = [&]() -> float {
    skip();
    char* end = nullptr;
    float v = std::strtof(p, &end);
    if (end) p = end;
    return v;
  };

  if (!match('{')) return false;
  for (;;) {
    skip();
    if (*p == '}') { ++p; return true; }
    std::string key = readStr();
    if (!match(':')) return false;
    if (key == "renderPath") out.renderPath = readStr();
    else if (key == "enableShadows") out.enableShadows = readBool();
    else if (key == "enableIBL") out.enableIBL = readBool();
    else if (key == "exposure") out.exposure = readFloat();
    if (!match(',') && *p != '}') return false;
  }
}
}  // namespace

class RenderingSettingsPanelImpl : public IRenderingSettingsPanel {
public:
  void Show() override { m_visible = true; }
  void Hide() override { m_visible = false; }
  bool IsVisible() const override { return m_visible; }
  RenderingConfig GetConfig() const override { return m_config; }
  void SetConfig(RenderingConfig const& config) override { m_config = config; }

  bool SaveConfig(char const* path) override {
    if (!path) return false;
    std::string json = ConfigToJson(m_config);
    return te::core::FileWrite(std::string(path), json);
  }

  bool LoadConfig(char const* path) override {
    if (!path) return false;
    auto data = te::core::FileRead(std::string(path));
    if (!data || data->empty()) return false;
    std::string str(data->begin(), data->end());
    if (!str.empty() && str.back() != '\0') str += '\0';
    return JsonToConfig(str.c_str(), m_config);
  }

private:
  bool m_visible = false;
  RenderingConfig m_config;
};

IRenderingSettingsPanel* CreateRenderingSettingsPanel() {
  return new RenderingSettingsPanelImpl();
}

}  // namespace editor
}  // namespace te
