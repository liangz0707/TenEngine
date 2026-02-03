#ifndef TE_SHADER_CACHE_HPP
#define TE_SHADER_CACHE_HPP

#include <te/shader/handle.hpp>

namespace te::shader {

class IShaderCache {
public:
    virtual ~IShaderCache() = default;
    virtual bool LoadCache(char const* path) = 0;
    virtual bool SaveCache(char const* path) = 0;
    virtual void Invalidate(IShaderHandle* handle) = 0;
};

}  // namespace te::shader

#endif  // TE_SHADER_CACHE_HPP
