#ifndef TE_SHADER_DETAIL_HOT_RELOAD_IMPL_HPP
#define TE_SHADER_DETAIL_HOT_RELOAD_IMPL_HPP

#include <te/shader/compiler.hpp>
#include <te/shader/hot_reload.hpp>
#include <atomic>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace te::shader {

struct WatchedPath {
    std::string path;
    SourceChangedCallback callback;
    void* userData;
    std::filesystem::file_time_type lastMtime;
};

class ShaderHotReloadImpl : public IShaderHotReload {
public:
    ShaderHotReloadImpl(IShaderCompiler* compiler, IShaderCache* cache);
    ~ShaderHotReloadImpl() override;
    bool ReloadShader(IShaderHandle* handle) override;
    void OnSourceChanged(char const* path, SourceChangedCallback callback, void* userData = nullptr) override;
    void NotifyShaderUpdated(IShaderHandle* handle) override;

private:
    void watchThreadFunc();

    IShaderCompiler* compiler_;
    IShaderCache* cache_;
    std::vector<WatchedPath> watched_;
    std::mutex watchedMutex_;
    std::atomic<bool> stop_{false};
    std::thread watchThread_;
};

}  // namespace te::shader

#endif
