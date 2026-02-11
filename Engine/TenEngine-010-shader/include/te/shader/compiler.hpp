#ifndef TE_SHADER_COMPILER_HPP
#define TE_SHADER_COMPILER_HPP

#include <te/shader/handle.hpp>
#include <te/shader/types.hpp>
#include <cstddef>

namespace te::shader {

class IShaderCache;

class IShaderCompiler {
public:
    virtual ~IShaderCompiler() = default;

    virtual IShaderHandle* LoadSource(char const* path, ShaderSourceFormat format) = 0;
    virtual IShaderHandle* LoadSourceFromMemory(void const* data, size_t size, ShaderSourceFormat format) = 0;
    virtual void ReleaseHandle(IShaderHandle* handle) = 0;

    virtual bool Compile(IShaderHandle* handle, CompileOptions const& options) = 0;
    virtual void const* GetBytecode(IShaderHandle* handle, size_t* out_size) = 0;
    /** Compile for the given stage and return bytecode. Result is valid until next GetBytecodeForStage or Compile. Does not change handle variant selection. */
    virtual void const* GetBytecodeForStage(IShaderHandle* handle, ShaderStage stage, size_t* out_size) { (void)handle; (void)stage; if (out_size) *out_size = 0; return nullptr; }
    virtual char const* GetLastError() const = 0;
    virtual BackendType GetTargetBackend() const = 0;

    virtual void DefineKeyword(char const* name, char const* value) = 0;
    virtual void EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out) = 0;
    virtual bool Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count) = 0;
    virtual void SetCache(IShaderCache* cache) {}

    /** Fill UniformLayoutDesc for 009-rendercore integration. Requires te_rendercore. \a outDesc points to te::rendercore::UniformLayoutDesc. Returns false if no SPIR-V or reflection unavailable. */
    virtual bool GetReflection(IShaderHandle* handle, void* outDesc) { (void)handle; (void)outDesc; return false; }

    /** Fill ShaderReflectionDesc (Uniform + Texture + Sampler). \a outDesc points to te::rendercore::ShaderReflectionDesc. Returns false if no SPIR-V or reflection unavailable. */
    virtual bool GetShaderReflection(IShaderHandle* handle, void* outDesc) { (void)handle; (void)outDesc; return false; }

    /** Fill VertexFormatDesc from vertex stage inputs (SPIR-V only). \a outDesc points to te::rendercore::VertexFormatDesc. Returns false if not a vertex shader or no inputs. */
    virtual bool GetVertexInputReflection(IShaderHandle* handle, void* outDesc) { (void)handle; (void)outDesc; return false; }
};

}  // namespace te::shader

#endif  // TE_SHADER_COMPILER_HPP
