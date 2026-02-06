#ifndef TE_SHADER_HOT_RELOAD_HPP
#define TE_SHADER_HOT_RELOAD_HPP

#include <te/shader/handle.hpp>
#include <te/shader/types.hpp>

namespace te::shader {

class IShaderCompiler;
class IShaderCache;

class IShaderHotReload {
public:
    virtual ~IShaderHotReload() = default;
    virtual bool ReloadShader(IShaderHandle* handle) = 0;
    virtual void OnSourceChanged(char const* path, SourceChangedCallback callback, void* userData = nullptr) = 0;
    virtual void NotifyShaderUpdated(IShaderHandle* handle) = 0;
};

}  // namespace te::shader

#endif  // TE_SHADER_HOT_RELOAD_HPP
