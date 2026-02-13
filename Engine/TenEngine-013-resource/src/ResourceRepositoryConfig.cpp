/**
 * @file ResourceRepositoryConfig.cpp
 * @brief Load/save asset/repositories.json (minimal JSON).
 */
#include <te/resource/ResourceRepositoryConfig.h>
#include <te/core/platform.h>
#include <cstring>
#include <sstream>
#include <filesystem>
#include <system_error>

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

static bool UnescapeJsonString(std::string const& in, std::string& out) {
  out.clear();
  for (size_t i = 0; i < in.size(); ++i) {
    if (in[i] == '\\' && i + 1 < in.size()) {
      switch (in[i + 1]) {
        case '"': out += '"'; ++i; break;
        case '\\': out += '\\'; ++i; break;
        case 'n': out += '\n'; ++i; break;
        case 'r': out += '\r'; ++i; break;
        case 't': out += '\t'; ++i; break;
        default: out += in[i]; break;
      }
    } else {
      out += in[i];
    }
  }
  return true;
}

static bool ParseStringValue(char const* start, char const* end, std::string& out) {
  if (end <= start || *start != '"') return false;
  ++start;
  std::string raw;
  while (start < end && *start != '"') {
    if (*start == '\\' && start + 1 < end) {
      raw += *start;
      raw += *(start + 1);
      start += 2;
      continue;
    }
    raw += *start++;
  }
  return UnescapeJsonString(raw, out);
}

/** Find end of JSON string starting at start (on opening "). Returns index past closing ", or npos. */
static size_t FindJsonStringEnd(std::string const& content, size_t start) {
  if (start >= content.size() || content[start] != '"') return std::string::npos;
  for (size_t j = start + 1; j < content.size(); ) {
    if (content[j] == '\\') { j += 2; continue; }
    if (content[j] == '"') return j + 1;
    ++j;
  }
  return std::string::npos;
}

/** Extract and unescape a JSON string value; start points to opening ". Returns true and advances *inout_pos past the string. */
static bool ExtractJsonString(std::string const& content, size_t* inout_pos, std::string& out) {
  size_t i = *inout_pos;
  if (i >= content.size() || content[i] != '"') return false;
  size_t end = FindJsonStringEnd(content, i);
  if (end == std::string::npos) return false;
  std::string raw = content.substr(i + 1, end - 1 - (i + 1));
  if (!UnescapeJsonString(raw, out)) return false;
  *inout_pos = end;
  return true;
}

bool LoadRepositoryConfig(char const* assetRoot, RepositoryConfig& out) {
  out.repositories.clear();
  if (!assetRoot || !*assetRoot) return false;
  std::string path = te::core::PathJoin(assetRoot, "repositories.json");
  auto data = te::core::FileRead(path);
  if (!data || data->empty()) return false;
  std::string content(data->begin(), data->end());
  size_t reposKey = content.find("\"repositories\"");
  if (reposKey == std::string::npos) return false;
  size_t pos = content.find('{', reposKey);
  if (pos == std::string::npos) return false;
  size_t i = pos + 1;
  while (i < content.size()) {
    while (i < content.size() && (content[i] == ' ' || content[i] == '\t' || content[i] == '\n' || content[i] == '\r' || content[i] == ',')) ++i;
    if (i >= content.size() || content[i] == '}') break;
    if (content[i] != '"') return false;
    size_t keyStart = i;
    std::string repoName;
    if (!ExtractJsonString(content, &keyStart, repoName)) return false;
    i = keyStart;
    while (i < content.size() && content[i] != '{') ++i;
    if (i >= content.size()) return false;
    size_t objStart = i;
    int brace = 1;
    ++i;
    while (i < content.size() && brace > 0) {
      if (content[i] == '"') {
        size_t end = FindJsonStringEnd(content, i);
        i = (end == std::string::npos) ? content.size() : end;
        continue;
      }
      if (content[i] == '{') ++brace;
      else if (content[i] == '}') --brace;
      ++i;
    }
    size_t objEnd = i;
    std::string objStr = content.substr(objStart, objEnd - objStart);

    RepositoryInfo info;
    info.name = repoName;

    size_t rootKey = objStr.find("\"root\"");
    if (rootKey != std::string::npos) {
      size_t colon = objStr.find(':', rootKey);
      if (colon != std::string::npos) {
        size_t valStart = objStr.find('"', colon);
        if (valStart != std::string::npos) {
          size_t valPos = valStart;
          if (ExtractJsonString(objStr, &valPos, info.root)) {}
        }
      }
    }
    size_t vpKey = objStr.find("\"virtualPrefix\"");
    if (vpKey != std::string::npos) {
      size_t colon = objStr.find(':', vpKey);
      if (colon != std::string::npos) {
        size_t valStart = objStr.find('"', colon);
        if (valStart != std::string::npos) {
          size_t valPos = valStart;
          if (ExtractJsonString(objStr, &valPos, info.virtualPrefix)) {}
        }
      }
    }
    if (info.root.empty()) info.root = repoName;
    if (info.virtualPrefix.empty()) info.virtualPrefix = repoName;
    out.repositories.push_back(info);
  }
  return true;
}

bool SaveRepositoryConfig(char const* assetRoot, RepositoryConfig const& config) {
  if (!assetRoot || !*assetRoot) return false;
  std::string path = te::core::PathJoin(assetRoot, "repositories.json");
  std::ostringstream oss;
  oss << "{\"repositories\":{";
  for (size_t i = 0; i < config.repositories.size(); ++i) {
    if (i) oss << ",";
    RepositoryInfo const& r = config.repositories[i];
    oss << "\"" << EscapeJsonString(r.name) << "\":{"
        << "\"root\":\"" << EscapeJsonString(r.root) << "\","
        << "\"virtualPrefix\":\"" << EscapeJsonString(r.virtualPrefix) << "\"}";
  }
  oss << "}}";
  return te::core::FileWrite(path, oss.str());
}

bool AddRepository(char const* assetRoot, char const* name) {
  if (!assetRoot || !*assetRoot || !name || !name[0]) return false;
  for (char const* p = name; *p; ++p) {
    char c = *p;
    if (c == '/' || c == '\\' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') return false;
  }
  RepositoryConfig config;
  LoadRepositoryConfig(assetRoot, config);
  for (auto const& r : config.repositories)
    if (r.name == name) return false;
  RepositoryInfo info;
  info.name = name;
  info.root = name;
  info.virtualPrefix = name;
  config.repositories.push_back(info);
  std::string repoDir = te::core::PathJoin(assetRoot, name);
  std::error_code ec;
  std::filesystem::create_directories(std::filesystem::u8path(repoDir), ec);
  if (ec) return false;
  if (!SaveRepositoryConfig(assetRoot, config)) return false;
  return true;
}

}  // namespace resource
}  // namespace te
