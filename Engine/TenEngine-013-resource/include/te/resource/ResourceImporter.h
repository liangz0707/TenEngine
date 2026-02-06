// 013-Resource: IResourceImporter interface per ABI (te/resource/ResourceImporter.h)
#pragma once

namespace te {
namespace resource {

class IResourceImporter {
public:
    virtual ~IResourceImporter() = default;
    // DetectFormat, Convert, Metadata, Dependencies — per contract; modules implement.
    virtual bool DetectFormat(char const* path) = 0;
    virtual bool Convert(char const* path, void* out_metadata_or_null) = 0;
};

} // namespace resource
} // namespace te
