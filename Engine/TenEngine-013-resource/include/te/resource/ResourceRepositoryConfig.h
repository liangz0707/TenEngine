/**
 * @file ResourceRepositoryConfig.h
 * @brief Repository configuration load/save for asset/repositories.json.
 */
#ifndef TE_RESOURCE_RESOURCE_REPOSITORY_CONFIG_H
#define TE_RESOURCE_RESOURCE_REPOSITORY_CONFIG_H

#include <string>
#include <vector>

namespace te {
namespace resource {

struct RepositoryInfo {
  std::string name;
  std::string root;
  std::string virtualPrefix;
};

struct RepositoryConfig {
  std::vector<RepositoryInfo> repositories;
};

/** Load repositories from assetRoot/repositories.json. Returns false on file missing or parse error. */
bool LoadRepositoryConfig(char const* assetRoot, RepositoryConfig& out);

/** Save repositories to assetRoot/repositories.json. */
bool SaveRepositoryConfig(char const* assetRoot, RepositoryConfig const& config);

/** Add a repository and create its root directory. Returns false if name exists or invalid. */
bool AddRepository(char const* assetRoot, char const* name);

}  // namespace resource
}  // namespace te

#endif
