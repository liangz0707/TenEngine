#ifndef TE_SHADER_DETAIL_COMPILER_IMPL_HPP
#define TE_SHADER_DETAIL_COMPILER_IMPL_HPP

#include <te/shader/compiler.hpp>
#include <te/shader/detail/handle_impl.hpp>
#include <te/shader/types.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace te::shader {
class ShaderHandleImpl;

class ShaderCacheImpl;

class ShaderCompilerImpl : public IShaderCompiler {
public:
    std::string lastError_;
    std::unordered_map<std::string, std::string> keywords_;
    BackendType targetBackend_ = BackendType::SPIRV;
    CompileOptions lastOptions_{};
    std::vector<std::unique_ptr<ShaderHandleImpl>> handles_;
    ShaderCacheImpl* cache_ = nullptr;
    std::vector<uint8_t> bytecodeForStageBuffer_;  /* result of GetBytecodeForStage, invalidated on next call */

    void SetCache(IShaderCache* cache) override;

    IShaderHandle* LoadSource(char const* path, ShaderSourceFormat format) override;
    IShaderHandle* LoadSourceFromMemory(void const* data, size_t size, ShaderSourceFormat format) override;
    void ReleaseHandle(IShaderHandle* handle) override;
    bool Compile(IShaderHandle* handle, CompileOptions const& options) override;
    void const* GetBytecode(IShaderHandle* handle, size_t* out_size) override;
    void const* GetBytecodeForStage(IShaderHandle* handle, ShaderStage stage, size_t* out_size) override;
    char const* GetLastError() const override;
    BackendType GetTargetBackend() const override;
    void DefineKeyword(char const* name, char const* value) override;
    void EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out) override;
    bool Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count) override;
    bool GetReflection(IShaderHandle* handle, void* outDesc) override;
    bool GetShaderReflection(IShaderHandle* handle, void* outDesc) override;
    bool GetVertexInputReflection(IShaderHandle* handle, void* outDesc) override;
};

}  // namespace te::shader

#endif
