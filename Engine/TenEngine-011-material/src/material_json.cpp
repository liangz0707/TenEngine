/**
 * @file material_json.cpp
 * @brief Parse and serialize .material JSON (shader, textures, parameters).
 */
#include <te/material/material_json.hpp>
#include <te/core/platform.h>
#include <cctype>
#include <cstring>
#include <sstream>

namespace te {
namespace material {

namespace {

void SkipSpace(char const*& p, char const* end) {
  while (p < end && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p;
}

bool Match(char const*& p, char const* end, char c) {
  SkipSpace(p, end);
  if (p < end && *p == c) { ++p; return true; }
  return false;
}

bool MatchString(char const*& p, char const* end, char const* str) {
  SkipSpace(p, end);
  size_t len = std::strlen(str);
  if (static_cast<size_t>(end - p) >= len && std::strncmp(p, str, len) == 0) {
    p += len;
    return true;
  }
  return false;
}

bool ReadQuotedString(char const*& p, char const* end, std::string& out) {
  SkipSpace(p, end);
  if (p >= end || *p != '"') return false;
  ++p;
  out.clear();
  while (p < end && *p != '"') {
    if (*p == '\\') {
      ++p;
      if (p >= end) return false;
      if (*p == 'n') out += '\n';
      else if (*p == 't') out += '\t';
      else if (*p == 'r') out += '\r';
      else out += *p;
      ++p;
    } else {
      out += *p++;
    }
  }
  if (p >= end || *p != '"') return false;
  ++p;
  return true;
}

bool ReadNumber(char const*& p, char const* end, double& out) {
  SkipSpace(p, end);
  if (p >= end) return false;
  char* endPtr = nullptr;
  out = std::strtod(p, &endPtr);
  if (endPtr == p) return false;
  p = endPtr;
  return true;
}

bool ParseParametersValue(char const*& p, char const* end, MaterialParamValue& out) {
  SkipSpace(p, end);
  if (p >= end) return false;
  out.values.clear();
  if (*p == '[') {
    ++p;
    for (;;) {
      SkipSpace(p, end);
      if (p < end && *p == ']') { ++p; break; }
      double v = 0;
      if (!ReadNumber(p, end, v)) return false;
      out.values.push_back(v);
      SkipSpace(p, end);
      if (p < end && *p == ',') { ++p; continue; }
      if (p < end && *p == ']') { ++p; break; }
      return false;
    }
  } else {
    double v = 0;
    if (!ReadNumber(p, end, v)) return false;
    out.values.push_back(v);
  }
  return true;
}

}  // namespace

bool ParseMaterialJSONFromMemory(char const* data, std::size_t size, MaterialJSONData& out) {
  out = {};
  if (!data) return false;
  char const* p = data;
  char const* end = data + size;

  if (!Match(p, end, '{')) return false;

  while (p < end) {
    SkipSpace(p, end);
    if (p < end && *p == '}') { ++p; break; }
    std::string key;
    if (!ReadQuotedString(p, end, key)) return false;
    if (!Match(p, end, ':')) return false;

    if (key == "guid") {
      if (!ReadQuotedString(p, end, out.guid)) return false;
    } else if (key == "shader") {
      if (!ReadQuotedString(p, end, out.shader)) return false;
    } else if (key == "textures") {
      if (!Match(p, end, '{')) return false;
      for (;;) {
        SkipSpace(p, end);
        if (p < end && *p == '}') { ++p; break; }
        std::string texName;
        if (!ReadQuotedString(p, end, texName)) return false;
        if (!Match(p, end, ':')) return false;
        std::string guid;
        if (!ReadQuotedString(p, end, guid)) return false;
        out.textures[texName] = guid;
        SkipSpace(p, end);
        if (p < end && *p == ',') { ++p; continue; }
        if (p < end && *p == '}') { ++p; break; }
        return false;
      }
    } else if (key == "parameters") {
      if (!Match(p, end, '{')) return false;
      for (;;) {
        SkipSpace(p, end);
        if (p < end && *p == '}') { ++p; break; }
        std::string paramName;
        if (!ReadQuotedString(p, end, paramName)) return false;
        if (!Match(p, end, ':')) return false;
        MaterialParamValue val;
        if (!ParseParametersValue(p, end, val)) return false;
        out.parameters[paramName] = std::move(val);
        SkipSpace(p, end);
        if (p < end && *p == ',') { ++p; continue; }
        if (p < end && *p == '}') { ++p; break; }
        return false;
      }
    } else {
      /* skip unknown key value */
      SkipSpace(p, end);
      if (p < end && *p == '"') {
        std::string dummy;
        if (!ReadQuotedString(p, end, dummy)) return false;
      } else if (p < end && (*p == '-' || std::isdigit(static_cast<unsigned char>(*p)))) {
        double d;
        if (!ReadNumber(p, end, d)) return false;
      } else if (p < end && *p == '[') {
        int depth = 1;
        ++p;
        while (p < end && depth > 0) {
          if (*p == '[') ++depth;
          else if (*p == ']') --depth;
          ++p;
        }
      } else if (p < end && *p == '{') {
        int depth = 1;
        ++p;
        while (p < end && depth > 0) {
          if (*p == '{') ++depth;
          else if (*p == '}') --depth;
          ++p;
        }
      } else
        return false;
    }
    SkipSpace(p, end);
    if (p < end && *p == ',') { ++p; continue; }
    if (p < end && *p == '}') { ++p; break; }
  }
  return true;
}

bool ParseMaterialJSON(char const* path, MaterialJSONData& out) {
  if (!path) return false;
  auto opt = te::core::FileRead(std::string(path));
  if (!opt.has_value() || opt->empty()) return false;
  std::string str(opt->begin(), opt->end());
  return ParseMaterialJSONFromMemory(str.c_str(), str.size(), out);
}

static void EscapeJSONString(std::ostream& os, std::string const& s) {
  for (char c : s) {
    if (c == '"') os << "\\\"";
    else if (c == '\\') os << "\\\\";
    else if (c == '\n') os << "\\n";
    else if (c == '\r') os << "\\r";
    else if (c == '\t') os << "\\t";
    else os << c;
  }
}

std::string SerializeMaterialJSONToString(MaterialJSONData const& data) {
  std::ostringstream os;
  os << "{\n";
  if (!data.guid.empty()) {
    os << "  \"guid\": \"";
    EscapeJSONString(os, data.guid);
    os << "\",\n";
  }
  os << "  \"shader\": \"";
  EscapeJSONString(os, data.shader);
  os << "\",\n  \"textures\": {";
  bool first = true;
  for (auto const& kv : data.textures) {
    if (!first) os << ", ";
    first = false;
    os << "\n    \"";
    EscapeJSONString(os, kv.first);
    os << "\": \"";
    EscapeJSONString(os, kv.second);
    os << "\"";
  }
  os << "\n  },\n  \"parameters\": {";
  first = true;
  for (auto const& kv : data.parameters) {
    if (!first) os << ", ";
    first = false;
    os << "\n    \"";
    EscapeJSONString(os, kv.first);
    os << "\": ";
    if (kv.second.values.size() == 1) {
      os << kv.second.values[0];
    } else {
      os << "[";
      for (size_t i = 0; i < kv.second.values.size(); ++i) {
        if (i) os << ", ";
        os << kv.second.values[i];
      }
      os << "]";
    }
  }
  os << "\n  }\n}\n";
  return os.str();
}

bool SerializeMaterialJSON(char const* path, MaterialJSONData const& data) {
  if (!path) return false;
  std::string json = SerializeMaterialJSONToString(data);
  return te::core::FileWrite(std::string(path), json);
}

}  // namespace material
}  // namespace te
