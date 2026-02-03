#include <te/shader/detail/hot_reload_impl.hpp>
#include <te/shader/detail/cache_impl.hpp>
#include <chrono>
#include <thread>

namespace te::shader {

ShaderHotReloadImpl::ShaderHotReloadImpl(IShaderCompiler* compiler, IShaderCache* cache)
    : compiler_(compiler), cache_(cache) {
    watchThread_ = std::thread(&ShaderHotReloadImpl::watchThreadFunc, this);
}

ShaderHotReloadImpl::~ShaderHotReloadImpl() {
    stop_ = true;
    if (watchThread_.joinable()) watchThread_.join();
}

bool ShaderHotReloadImpl::ReloadShader(IShaderHandle* handle) {
    if (!compiler_ || !handle) return false;
    if (cache_) {
        static_cast<ShaderCacheImpl*>(cache_)->Invalidate(handle);
    }
    return compiler_->Compile(handle, CompileOptions{});
}

void ShaderHotReloadImpl::OnSourceChanged(char const* path, SourceChangedCallback callback, void* userData) {
    if (!path || !callback) return;
    std::error_code ec;
    auto mtime = std::filesystem::last_write_time(path, ec);
    if (ec) return;
    std::lock_guard<std::mutex> lock(watchedMutex_);
    watched_.push_back({path, callback, userData, mtime});
}

void ShaderHotReloadImpl::NotifyShaderUpdated(IShaderHandle* handle) {
    (void)handle;
}

void ShaderHotReloadImpl::watchThreadFunc() {
    while (!stop_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::vector<WatchedPath> toInvoke;
        {
            std::lock_guard<std::mutex> lock(watchedMutex_);
            for (auto& w : watched_) {
                std::error_code ec;
                auto mtime = std::filesystem::last_write_time(w.path, ec);
                if (!ec && mtime != w.lastMtime) {
                    w.lastMtime = mtime;
                    toInvoke.push_back(w);
                }
            }
        }
        for (auto const& w : toInvoke) {
            if (w.callback) w.callback(w.path.c_str(), w.userData);
        }
    }
}

}  // namespace te::shader
