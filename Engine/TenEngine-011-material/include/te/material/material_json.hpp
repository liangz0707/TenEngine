/**
 * @file material_json.hpp
 * @brief .material file format: UTF-8 JSON with top-level "shader", "textures", "parameters".
 * Replaces binary storage_format for MaterialResource Load/Save.
 */
#ifndef TE_MATERIAL_MATERIAL_JSON_HPP
#define TE_MATERIAL_MATERIAL_JSON_HPP

#include <string>
#include <vector>
#include <map>

namespace te {
namespace material {

/** Parameter value: scalar or array of numbers (JSON number / number array). */
struct MaterialParamValue {
  std::vector<double> values;  /* 1 element = scalar */
};

/** In-memory representation of .material JSON. */
struct MaterialJSONData {
  std::string guid;   /* Optional: Material ResourceId as GUID string; empty = generate on Load */
  std::string shader;  /* Shader GUID string */
  std::map<std::string, std::string> textures;  /* binding name -> texture GUID string */
  std::map<std::string, MaterialParamValue> parameters;  /* uniform name -> value(s) */
};

/** Parse .material file (UTF-8 JSON). Returns false on parse error or invalid structure. */
bool ParseMaterialJSON(char const* path, MaterialJSONData& out);

/** Parse from memory (null-terminated UTF-8 JSON). */
bool ParseMaterialJSONFromMemory(char const* data, std::size_t size, MaterialJSONData& out);

/** Serialize to JSON and write to file. Returns false on write error. */
bool SerializeMaterialJSON(char const* path, MaterialJSONData const& data);

/** Serialize to JSON string (for testing or alternate write). */
std::string SerializeMaterialJSONToString(MaterialJSONData const& data);

}  // namespace material
}  // namespace te

#endif
