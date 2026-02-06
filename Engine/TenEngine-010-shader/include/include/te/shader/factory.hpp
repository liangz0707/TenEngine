#ifndef TE_SHADER_FACTORY_HPP
#define TE_SHADER_FACTORY_HPP

#include <te/shader/cache.hpp>
#include <te/shader/compiler.hpp>
#include <te/shader/hot_reload.hpp>

namespace te::shader {

IShaderCompiler* CreateShaderCompiler();
void DestroyShaderCompiler(IShaderCompiler* c);

IShaderCache* CreateShaderCache();
void DestroyShaderCache(IShaderCache* c);

IShaderHotReload* CreateShaderHotReload(IShaderCompiler* compiler, IShaderCache* cache);
void DestroyShaderHotReload(IShaderHotReload* h);

}  // namespace te::shader

#endif  // TE_SHADER_FACTORY_HPP
