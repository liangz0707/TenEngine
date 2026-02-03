#include <te/shader/api.hpp>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <chrono>
#include <fstream>
#include <string>
#include <thread>

int main() {
    te::shader::IShaderCompiler* compiler = te::shader::CreateShaderCompiler();
    te::shader::IShaderCache* cache = te::shader::CreateShaderCache();
    assert(compiler && cache && "Create failed");
    compiler->SetCache(cache);

    te::shader::IShaderHotReload* hotReload = te::shader::CreateShaderHotReload(compiler, cache);
    assert(hotReload && "CreateShaderHotReload failed");

    char const* path = "test_shader_hotreload.vert";
    {
        std::ofstream f(path);
        f << "#version 450\nvoid main() { gl_Position = vec4(0,0,0,1); }\n";
    }

    te::shader::IShaderHandle* handle = compiler->LoadSource(path, te::shader::ShaderSourceFormat::GLSL);
    assert(handle && "LoadSource failed");

    bool ok = compiler->Compile(handle, te::shader::CompileOptions{});
    assert(ok && "Compile failed");

    size_t size0 = 0;
    (void)compiler->GetBytecode(handle, &size0);
    assert(size0 > 0);

    bool callbackFired = false;
    hotReload->OnSourceChanged(path, [](char const* p, void* user) {
        (void)p;
        *static_cast<bool*>(user) = true;
    }, &callbackFired);

    {
        std::ofstream f(path);
        f << "#version 450\nvoid main() { gl_Position = vec4(1,0,0,1); }\n";
    }

    for (int i = 0; i < 20 && !callbackFired; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    assert(callbackFired && "OnSourceChanged callback should fire");

    ok = hotReload->ReloadShader(handle);
    assert(ok && "ReloadShader failed");

    size_t size1 = 0;
    void const* bc = compiler->GetBytecode(handle, &size1);
    assert(bc && size1 > 0 && "GetBytecode after reload");
    hotReload->NotifyShaderUpdated(handle);

    te::shader::DestroyShaderHotReload(hotReload);
    compiler->ReleaseHandle(handle);
    te::shader::DestroyShaderCache(cache);
    te::shader::DestroyShaderCompiler(compiler);

    std::remove(path);
    std::printf("te_shader test_hot_reload: all OK\n");
    return 0;
}
