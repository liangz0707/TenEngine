/**
 * @file ResourceManifest.h
 * @brief Per-repo manifest (guid, assetPath, type, repository, displayName, assetFolders) load/save.
 *        Storage path is computed internally; only asset path is persisted.
 */
#ifndef TE_RESOURCE_RESOURCE_MANIFEST_H
#define TE_RESOURCE_RESOURCE_MANIFEST_H

#include <te/resource/ResourceId.h>
#include <te/resource/ResourceTypes.h>
#include <string>
#include <vector>

namespace te {
namespace resource {

struct ManifestEntry {
  ResourceId guid;
  std::string assetPath;
  ResourceType type = ResourceType::Custom;
  std::string repository;
  std::string displayName;
};

struct ResourceManifest {
  std::vector<ManifestEntry> resources;
  std::vector<std::string> assetFolders;
};

/** Load manifest from file. Returns false on missing or parse error. */
bool LoadManifest(char const* manifestPath, ResourceManifest& out);

/** Save manifest to file. */
bool SaveManifest(char const* manifestPath, ResourceManifest const& manifest);

/** ResourceType to string for JSON. */
char const* ResourceTypeToString(ResourceType t);

/** Parse ResourceType from string. Returns ResourceType::_Count if unknown. */
ResourceType ResourceTypeFromString(char const* s);

}  // namespace resource
}  // namespace te

#endif
