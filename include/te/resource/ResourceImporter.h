/**
 * @file ResourceImporter.h
 * @brief IResourceImporter (contract: specs/_contracts/013-resource-ABI.md).
 */
#ifndef TE_RESOURCE_RESOURCE_IMPORTER_H
#define TE_RESOURCE_RESOURCE_IMPORTER_H

namespace te {
namespace resource {

/** Per-type importer; DetectFormat, Convert, output description/data, Metadata, Dependencies. */
class IResourceImporter {
 public:
  virtual ~IResourceImporter() = default;
  virtual bool DetectFormat(char const* path) = 0;
  virtual bool Convert(char const* path, void* out_description_or_null, void* out_metadata_or_null) = 0;
};

}  // namespace resource
}  // namespace te

#endif  // TE_RESOURCE_RESOURCE_IMPORTER_H
