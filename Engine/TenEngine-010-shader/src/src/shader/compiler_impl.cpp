#include <te/shader/detail/compiler_impl.hpp>
#include <te/shader/detail/cache_impl.hpp>
#include <te/shader/detail/glslang_backend.hpp>
#include <te/shader/detail/handle_impl.hpp>
#include <te/shader/detail/spirv_cross_backend.hpp>
#if defined(TE_RHI_D3D12) && TE_RHI_D3D12
#include <te/shader/detail/dxc_backend.hpp>
#endif
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
#include <te/core/platform.h>
#include <te/core/log.h>
#include <te/rendercore/uniform_layout.hpp>
#include <te/rendercore/shader_reflection.hpp>
#endif
#if defined(TENENGINE_USE_SPIRV_CROSS) && TENENGINE_USE_SPIRV_CROSS
#include <spirv_cross.hpp>
#endif
#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <string>

namespace te::shader {

#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE && defined(TENENGINE_USE_SPIRV_CROSS) && TENENGINE_USE_SPIRV_CROSS
static void ExtractReflectionFromSpirv(ShaderHandleImpl* impl);
#endif

IShaderHandle* ShaderCompilerImpl::LoadSource(char const* path, ShaderSourceFormat format) {
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    auto opt = te::core::FileRead(path ? std::string(path) : std::string());
    if (!opt || opt->empty()) {
        lastError_ = "File not found: ";
        lastError_ += path ? path : "(null)";
        te::core::Log(te::core::LogLevel::Error, lastError_.c_str());
        return nullptr;
    }
    std::string src(opt->begin(), opt->end());
    return LoadSourceFromMemory(src.data(), src.size(), format);
#else
    std::ifstream f(path);
    if (!f) {
        lastError_ = "File not found: ";
        lastError_ += path;
        return nullptr;
    }
    std::ostringstream oss;
    oss << f.rdbuf();
    return LoadSourceFromMemory(oss.str().data(), oss.str().size(), format);
#endif
}

IShaderHandle* ShaderCompilerImpl::LoadSourceFromMemory(void const* data, size_t size, ShaderSourceFormat format) {
    auto h = std::make_unique<ShaderHandleImpl>();
    h->sourceCode_.assign(static_cast<char const*>(data), size);
    h->sourceFormat_ = format;
    h->currentKey_.hash = 0;
    IShaderHandle* p = h.get();
    handles_.push_back(std::move(h));
    return p;
}

void ShaderCompilerImpl::ReleaseHandle(IShaderHandle* handle) {
    if (!handle) return;
    for (auto it = handles_.begin(); it != handles_.end(); ++it) {
        if (it->get() == handle) {
            handles_.erase(it);
            return;
        }
    }
}

bool ShaderCompilerImpl::Compile(IShaderHandle* handle, CompileOptions const& options) {
    if (!handle) return false;
    auto* impl = static_cast<ShaderHandleImpl*>(handle);
    targetBackend_ = options.targetBackend;

    if (cache_ && cache_->TryLoadToHandle(impl)) {
        if (impl->variantBytecode_.find(impl->currentKey_.hash) != impl->variantBytecode_.end())
            return true;
    }

    bool ok = false;
    if (impl->sourceFormat_ == ShaderSourceFormat::GLSL || impl->sourceFormat_ == ShaderSourceFormat::HLSL) {
        if (targetBackend_ == BackendType::SPIRV) {
            ok = CompileGlslToSpirv(impl, lastError_);
        } else if (targetBackend_ == BackendType::MSL || targetBackend_ == BackendType::HLSL_SOURCE) {
            if (!CompileGlslToSpirv(impl, lastError_)) return false;
            SpirvCrossTarget t = (targetBackend_ == BackendType::MSL) ? SpirvCrossTarget::MSL : SpirvCrossTarget::HLSL;
            ok = CompileSpirvToTarget(impl, t, lastError_);
        }
    }
#if defined(TE_RHI_D3D12) && TE_RHI_D3D12
    if (!ok && impl->sourceFormat_ == ShaderSourceFormat::HLSL && targetBackend_ == BackendType::DXIL) {
        ok = CompileHlslToDxil(impl, lastError_);
    }
#endif
    if (!ok && lastError_.empty()) {
        lastError_ = "Unsupported format/backend combination";
        return false;
    }
    if (ok) {
        VariantBytecode& vb = impl->variantBytecode_[impl->currentKey_.hash];
        vb.spirv = impl->bytecode_;
        vb.dxil = impl->bytecodeBlob_;
        vb.crossCompiled = impl->crossCompiledSource_;
        if (cache_) cache_->StoreFromHandle(impl);
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE && defined(TENENGINE_USE_SPIRV_CROSS) && TENENGINE_USE_SPIRV_CROSS
        ExtractReflectionFromSpirv(impl);
#endif
    }
    return ok;
}

void const* ShaderCompilerImpl::GetBytecode(IShaderHandle* handle, size_t* out_size) {
    if (!handle || !out_size) return nullptr;
    auto* impl = static_cast<ShaderHandleImpl*>(handle);
    auto it = impl->variantBytecode_.find(impl->currentKey_.hash);
    VariantBytecode const* vb = (it != impl->variantBytecode_.end()) ? &it->second : nullptr;
    if (targetBackend_ == BackendType::SPIRV) {
        auto const& spirv = vb ? vb->spirv : impl->bytecode_;
        if (spirv.empty()) return nullptr;
        *out_size = spirv.size() * sizeof(uint32_t);
        return spirv.data();
    }
    if (targetBackend_ == BackendType::DXIL) {
        auto const& dxil = vb ? vb->dxil : impl->bytecodeBlob_;
        if (dxil.empty()) return nullptr;
        *out_size = dxil.size();
        return dxil.data();
    }
    if (targetBackend_ == BackendType::MSL || targetBackend_ == BackendType::HLSL_SOURCE) {
        auto const& src = vb ? vb->crossCompiled : impl->crossCompiledSource_;
        if (src.empty()) return nullptr;
        *out_size = src.size();
        return src.data();
    }
    return nullptr;
}

char const* ShaderCompilerImpl::GetLastError() const { return lastError_.c_str(); }
BackendType ShaderCompilerImpl::GetTargetBackend() const { return targetBackend_; }

void ShaderCompilerImpl::SetCache(IShaderCache* cache) {
    cache_ = cache ? static_cast<ShaderCacheImpl*>(cache) : nullptr;
}

void ShaderCompilerImpl::DefineKeyword(char const* name, char const* value) {
    if (name && value) keywords_[name] = value;
}

namespace {

void collectIfdefMacros(std::string const& source, std::vector<std::string>& out) {
    static char const* patterns[] = { "#ifdef ", "#ifndef ", "#if defined(", "#if defined (" };
    for (size_t pos = 0; (pos = source.find('#', pos)) != std::string::npos; ++pos) {
        size_t rest = source.size() - pos;
        for (char const* pfx : patterns) {
            size_t len = std::strlen(pfx);
            if (rest >= len && source.compare(pos, len, pfx) == 0) {
                size_t start = pos + len;
                size_t end = start;
                if (std::strncmp(pfx, "#if defined", 10) == 0) {
                    while (end < source.size() && source[end] != ')' && source[end] != '\n') ++end;
                } else {
                    while (end < source.size() && (std::isalnum(static_cast<unsigned char>(source[end])) || source[end] == '_')) ++end;
                }
                if (end > start) {
                    std::string name = source.substr(start, end - start);
                    if (std::find(out.begin(), out.end(), name) == out.end())
                        out.push_back(std::move(name));
                }
                break;
            }
        }
    }
}

uint64_t hashMacroSet(MacroSet const& m) {
    uint64_t h = 0;
    for (uint32_t i = 0; i < m.count && i < MacroSet::kMaxPairs; ++i) {
        for (size_t j = 0; m.names[i][j] != '\0'; ++j)
            h = h * 31u + static_cast<unsigned char>(m.names[i][j]);
        h = h * 31u + '=';
        for (size_t j = 0; m.values[i][j] != '\0'; ++j)
            h = h * 31u + static_cast<unsigned char>(m.values[i][j]);
    }
    return h;
}

}  // anonymous namespace

#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE && defined(TENENGINE_USE_SPIRV_CROSS) && TENENGINE_USE_SPIRV_CROSS

static void ExtractReflectionFromSpirv(ShaderHandleImpl* impl) {
    if (!impl || impl->bytecode_.empty()) return;
    try {
        spirv_cross::Compiler comp(impl->bytecode_.data(), impl->bytecode_.size());
        auto res = comp.get_shader_resources();
        impl->reflectionMembers_.clear();
        impl->reflectionTotalSize_ = 0;
        impl->reflectionResourceBindings_.clear();

        // Uniform buffer members
        for (auto const& ub : res.uniform_buffers) {
            auto const& type = comp.get_type(ub.base_type_id);
            if (type.basetype != spirv_cross::SPIRType::Struct) continue;
            impl->reflectionTotalSize_ = static_cast<uint32_t>(comp.get_declared_struct_size(type));
            for (uint32_t i = 0; i < type.member_types.size(); ++i) {
                te::rendercore::UniformMember m{};
                std::string name = comp.get_member_name(type.self, i);
                if (name.size() >= 64) name.resize(63);
                std::memcpy(m.name, name.c_str(), name.size() + 1);
                m.offset = static_cast<uint32_t>(comp.get_member_decoration(type.self, i, spv::DecorationOffset));
                m.size = static_cast<uint32_t>(comp.get_declared_struct_member_size(type, i));
                auto const& mt = comp.get_type(type.member_types[i]);
                using BT = spirv_cross::SPIRType::BaseType;
                if (mt.basetype == BT::Float) {
                    if (mt.vecsize == 1 && mt.columns == 1) m.type = te::rendercore::UniformMemberType::Float;
                    else if (mt.vecsize == 2 && mt.columns == 1) m.type = te::rendercore::UniformMemberType::Float2;
                    else if (mt.vecsize == 3 && mt.columns == 1) m.type = te::rendercore::UniformMemberType::Float3;
                    else if (mt.vecsize == 4 && mt.columns == 1) m.type = te::rendercore::UniformMemberType::Float4;
                    else if (mt.vecsize == 3 && mt.columns == 3) m.type = te::rendercore::UniformMemberType::Mat3;
                    else if (mt.vecsize == 4 && mt.columns == 4) m.type = te::rendercore::UniformMemberType::Mat4;
                    else m.type = te::rendercore::UniformMemberType::Unknown;
                } else if (mt.basetype == BT::Int || mt.basetype == BT::UInt || mt.basetype == BT::Int64 || mt.basetype == BT::UInt64) {
                    if (mt.vecsize == 1) m.type = te::rendercore::UniformMemberType::Int;
                    else if (mt.vecsize == 2) m.type = te::rendercore::UniformMemberType::Int2;
                    else if (mt.vecsize == 3) m.type = te::rendercore::UniformMemberType::Int3;
                    else if (mt.vecsize == 4) m.type = te::rendercore::UniformMemberType::Int4;
                    else m.type = te::rendercore::UniformMemberType::Unknown;
                } else {
                    m.type = te::rendercore::UniformMemberType::Unknown;
                }
                impl->reflectionMembers_.push_back(m);
            }
            break;
        }

        // Sampled images (Textures)
        for (auto const& img : res.sampled_images) {
            te::rendercore::ShaderResourceBinding b{};
            if (img.name.size() >= 64) {
                std::memcpy(b.name, img.name.c_str(), 63);
                b.name[63] = '\0';
            } else {
                std::memcpy(b.name, img.name.c_str(), img.name.size() + 1);
            }
            b.kind = te::rendercore::ShaderResourceKind::SampledImage;
            b.set = comp.has_decoration(img.id, spv::DecorationDescriptorSet)
                ? static_cast<uint32_t>(comp.get_decoration(img.id, spv::DecorationDescriptorSet))
                : 0u;
            b.binding = comp.has_decoration(img.id, spv::DecorationBinding)
                ? static_cast<uint32_t>(comp.get_decoration(img.id, spv::DecorationBinding))
                : 0u;
            impl->reflectionResourceBindings_.push_back(b);
        }

        // Separate images (Vulkan separate image/sampler)
        for (auto const& img : res.separate_images) {
            te::rendercore::ShaderResourceBinding b{};
            if (img.name.size() >= 64) {
                std::memcpy(b.name, img.name.c_str(), 63);
                b.name[63] = '\0';
            } else {
                std::memcpy(b.name, img.name.c_str(), img.name.size() + 1);
            }
            b.kind = te::rendercore::ShaderResourceKind::SampledImage;
            b.set = comp.has_decoration(img.id, spv::DecorationDescriptorSet)
                ? static_cast<uint32_t>(comp.get_decoration(img.id, spv::DecorationDescriptorSet))
                : 0u;
            b.binding = comp.has_decoration(img.id, spv::DecorationBinding)
                ? static_cast<uint32_t>(comp.get_decoration(img.id, spv::DecorationBinding))
                : 0u;
            impl->reflectionResourceBindings_.push_back(b);
        }

        // Samplers
        for (auto const& s : res.separate_samplers) {
            te::rendercore::ShaderResourceBinding b{};
            if (s.name.size() >= 64) {
                std::memcpy(b.name, s.name.c_str(), 63);
                b.name[63] = '\0';
            } else {
                std::memcpy(b.name, s.name.c_str(), s.name.size() + 1);
            }
            b.kind = te::rendercore::ShaderResourceKind::Sampler;
            b.set = comp.has_decoration(s.id, spv::DecorationDescriptorSet)
                ? static_cast<uint32_t>(comp.get_decoration(s.id, spv::DecorationDescriptorSet))
                : 0u;
            b.binding = comp.has_decoration(s.id, spv::DecorationBinding)
                ? static_cast<uint32_t>(comp.get_decoration(s.id, spv::DecorationBinding))
                : 0u;
            impl->reflectionResourceBindings_.push_back(b);
        }
    } catch (...) {}
}

#endif

void ShaderCompilerImpl::EnumerateVariants(IShaderHandle* handle, IVariantEnumerator* out) {
    if (!handle || !out) return;
    auto* impl = static_cast<ShaderHandleImpl*>(handle);
    impl->variantMacros_.clear();
    std::vector<std::string> macroNames;
    collectIfdefMacros(impl->sourceCode_, macroNames);
    if (macroNames.empty() || macroNames.size() > 16) {
        VariantKey k;
        k.hash = 0;
        impl->variantMacros_[0] = impl->macros_;
        out->OnVariant(k);
        return;
    }
    size_t n = macroNames.size();
    size_t total = size_t(1) << n;
    for (size_t bits = 0; bits < total; ++bits) {
        MacroSet ms{};
        for (size_t i = 0; i < n && ms.count < MacroSet::kMaxPairs; ++i) {
            size_t nameLen = std::min(macroNames[i].size(), size_t(63));
            std::memcpy(ms.names[ms.count], macroNames[i].c_str(), nameLen + 1);
            std::memcpy(ms.values[ms.count], (bits & (size_t(1) << i)) ? "1" : "0", 2);
            ++ms.count;
        }
        VariantKey k;
        k.hash = hashMacroSet(ms);
        impl->variantMacros_[k.hash] = ms;
        out->OnVariant(k);
    }
}

bool ShaderCompilerImpl::GetReflection(IShaderHandle* handle, void* outDesc) {
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    if (!handle || !outDesc) return false;
    auto* impl = static_cast<ShaderHandleImpl*>(handle);
    if (impl->reflectionMembers_.empty()) return false;
    auto* desc = static_cast<te::rendercore::UniformLayoutDesc*>(outDesc);
    desc->members = impl->reflectionMembers_.data();
    desc->memberCount = static_cast<uint32_t>(impl->reflectionMembers_.size());
    desc->totalSize = impl->reflectionTotalSize_;
    return true;
#else
    (void)handle;
    (void)outDesc;
    return false;
#endif
}

bool ShaderCompilerImpl::GetShaderReflection(IShaderHandle* handle, void* outDesc) {
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    if (!handle || !outDesc) return false;
    auto* impl = static_cast<ShaderHandleImpl*>(handle);
    // Success if we have uniform block OR resource bindings
    bool hasUniform = !impl->reflectionMembers_.empty();
    bool hasResources = !impl->reflectionResourceBindings_.empty();
    if (!hasUniform && !hasResources) return false;
    auto* desc = static_cast<te::rendercore::ShaderReflectionDesc*>(outDesc);
    if (hasUniform) {
        desc->uniformBlock.members = impl->reflectionMembers_.data();
        desc->uniformBlock.memberCount = static_cast<uint32_t>(impl->reflectionMembers_.size());
        desc->uniformBlock.totalSize = impl->reflectionTotalSize_;
    } else {
        desc->uniformBlock = {};
    }
    if (hasResources) {
        desc->resourceBindings = impl->reflectionResourceBindings_.data();
        desc->resourceBindingCount = static_cast<uint32_t>(impl->reflectionResourceBindings_.size());
    } else {
        desc->resourceBindings = nullptr;
        desc->resourceBindingCount = 0;
    }
    return true;
#else
    (void)handle;
    (void)outDesc;
    return false;
#endif
}

bool ShaderCompilerImpl::Precompile(IShaderHandle* handle, VariantKey const* keys, size_t count) {
    if (!handle) return false;
    if (count == 0 || !keys) {
        lastError_ = "Precompile: empty or invalid VariantKey list";
        return false;
    }
    auto* impl = static_cast<ShaderHandleImpl*>(handle);
    for (size_t i = 0; i < count; ++i) {
        impl->SelectVariant(keys[i]);
        if (!Compile(handle, CompileOptions{})) return false;
    }
    return true;
}

}  // namespace te::shader
