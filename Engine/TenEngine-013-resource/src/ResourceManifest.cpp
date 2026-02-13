/**
 * @file ResourceManifest.cpp
 * @brief Load/save per-repo manifest.json (minimal JSON).
 */
#include <te/resource/ResourceManifest.h>
#include <te/core/platform.h>
#include <te/object/Guid.h>
#include <cstring>
#include <sstream>
#include <algorithm>

namespace te {
namespace resource {

static std::string EscapeJsonString(std::string const& s) {
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

char const* ResourceTypeToString(ResourceType t) {
  switch (t) {
    case ResourceType::Texture: return "Texture";
    case ResourceType::Mesh: return "Mesh";
    case ResourceType::Material: return "Material";
    case ResourceType::Model: return "Model";
    case ResourceType::Effect: return "Effect";
    case ResourceType::Terrain: return "Terrain";
    case ResourceType::Shader: return "Shader";
    case ResourceType::Audio: return "Audio";
    case ResourceType::Level: return "Level";
    case ResourceType::Custom: return "Custom";
    default: return "Custom";
  }
}

ResourceType ResourceTypeFromString(char const* s) {
  if (!s) return ResourceType::_Count;
  if (std::strcmp(s, "Texture") == 0) return ResourceType::Texture;
  if (std::strcmp(s, "Mesh") == 0) return ResourceType::Mesh;
  if (std::strcmp(s, "Material") == 0) return ResourceType::Material;
  if (std::strcmp(s, "Model") == 0) return ResourceType::Model;
  if (std::strcmp(s, "Effect") == 0) return ResourceType::Effect;
  if (std::strcmp(s, "Terrain") == 0) return ResourceType::Terrain;
  if (std::strcmp(s, "Shader") == 0) return ResourceType::Shader;
  if (std::strcmp(s, "Audio") == 0) return ResourceType::Audio;
  if (std::strcmp(s, "Level") == 0) return ResourceType::Level;
  if (std::strcmp(s, "Custom") == 0) return ResourceType::Custom;
  return ResourceType::_Count;
}

static void ParseResourceEntry(std::string const& obj, ManifestEntry& e) {
  auto extract = [&obj](char const* key, std::string& out) {
    std::string search = "\"";
    search += key;
    search += "\"";
    size_t pos = obj.find(search);
    if (pos == std::string::npos) return;
    pos = obj.find(':', pos);
    if (pos == std::string::npos) return;
    pos = obj.find('"', pos);
    if (pos == std::string::npos) return;
    size_t start = pos + 1;
    size_t end = start;
    while (end < obj.size() && obj[end] != '"') {
      if (obj[end] == '\\') ++end;
      ++end;
    }
    if (end <= obj.size()) out = obj.substr(start, end - start);
  };
  std::string guidStr, typeStr;
  extract("guid", guidStr);
  extract("assetPath", e.assetPath);
  extract("type", typeStr);
  extract("repository", e.repository);
  extract("displayName", e.displayName);
  if (!guidStr.empty()) e.guid = ResourceId(object::GUID::FromString(guidStr.c_str()));
  e.type = ResourceTypeFromString(typeStr.c_str());
}

bool LoadManifest(char const* manifestPath, ResourceManifest& out) {
  out.resources.clear();
  out.assetFolders.clear();
  if (!manifestPath) return false;
  auto data = te::core::FileRead(manifestPath);
  if (!data || data->empty()) return false;
  std::string content(data->begin(), data->end());

  size_t resPos = content.find("\"resources\"");
  if (resPos != std::string::npos) {
    size_t arrStart = content.find('[', resPos);
    if (arrStart != std::string::npos) {
      size_t i = arrStart + 1;
      while (i < content.size()) {
        while (i < content.size() && (content[i] == ' ' || content[i] == '\t' || content[i] == '\n' || content[i] == '\r' || content[i] == ',')) ++i;
        if (i >= content.size() || content[i] == ']') break;
        if (content[i] != '{') { ++i; continue; }
        size_t objStart = i;
        int brace = 1;
        ++i;
        while (i < content.size() && brace > 0) {
          if (content[i] == '"') {
            ++i;
            while (i < content.size()) {
              if (content[i] == '\\') { i += 2; continue; }
              if (content[i] == '"') break;
              ++i;
            }
            if (i < content.size()) ++i;
            continue;
          }
          if (content[i] == '{') ++brace;
          else if (content[i] == '}') --brace;
          ++i;
        }
        std::string objStr = content.substr(objStart, i - objStart);
        ManifestEntry e;
        ParseResourceEntry(objStr, e);
        if (!e.guid.IsNull())
          out.resources.push_back(e);
      }
    }
  }

  size_t afPos = content.find("\"assetFolders\"");
  if (afPos != std::string::npos) {
    size_t arrStart = content.find('[', afPos);
    if (arrStart != std::string::npos) {
      size_t i = arrStart + 1;
      while (i < content.size()) {
        while (i < content.size() && (content[i] == ' ' || content[i] == '\t' || content[i] == '\n' || content[i] == '\r' || content[i] == ',')) ++i;
        if (i >= content.size() || content[i] == ']') break;
        if (content[i] != '"') { ++i; continue; }
        size_t start = i + 1;
        i = start;
        std::string folder;
        while (i < content.size()) {
          if (content[i] == '\\' && i + 1 < content.size()) { folder += content[i + 1]; i += 2; continue; }
          if (content[i] == '"') break;
          folder += content[i++];
        }
        if (i < content.size()) ++i;
        if (!folder.empty()) out.assetFolders.push_back(folder);
      }
    }
  }
  return true;
}

bool SaveManifest(char const* manifestPath, ResourceManifest const& manifest) {
  if (!manifestPath) return false;
  std::ostringstream oss;
  oss << "{\"resources\":[";
  for (size_t i = 0; i < manifest.resources.size(); ++i) {
    if (i) oss << ",";
    ManifestEntry const& e = manifest.resources[i];
    oss << "{\"guid\":\"" << EscapeJsonString(e.guid.ToString()) << "\","
        << "\"assetPath\":\"" << EscapeJsonString(e.assetPath) << "\","
        << "\"type\":\"" << ResourceTypeToString(e.type) << "\","
        << "\"repository\":\"" << EscapeJsonString(e.repository) << "\","
        << "\"displayName\":\"" << EscapeJsonString(e.displayName) << "\"}";
  }
  oss << "],\"assetFolders\":[";
  for (size_t i = 0; i < manifest.assetFolders.size(); ++i) {
    if (i) oss << ",";
    oss << "\"" << EscapeJsonString(manifest.assetFolders[i]) << "\"";
  }
  oss << "]}";
  return te::core::FileWrite(manifestPath, oss.str());
}

}  // namespace resource
}  // namespace te
