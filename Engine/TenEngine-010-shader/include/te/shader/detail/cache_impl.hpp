#ifndef TE_SHADER_DETAIL_CACHE_IMPL_HPP
#define TE_SHADER_DETAIL_CACHE_IMPL_HPP

#include <te/shader/cache.hpp>
#include <te/shader/detail/handle_impl.hpp>
#include <te/shader/types.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace te::shader {

class ShaderCacheImpl : public IShaderCache {
public:
    bool LoadCache(char const* path) override;
    bool SaveCache(char const* path) override;
    void Invalidate(IShaderHandle* handle) override;

    void StoreFromHandle(ShaderHandleImpl* handle);
    bool TryLoadToHandle(ShaderHandleImpl* handle);

private:
    using Key = std::string;
    std::unordered_map<Key, std::unordered_map<uint64_t, VariantBytecode>> store_;
};

}  // namespace te::shader

#endif
