#include <te/shader/detail/cache_impl.hpp>
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
#include <te/core/platform.h>
#include <te/core/log.h>
#endif
#include <cstring>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>

namespace te::shader {

namespace {
uint64_t hashSource(char const* path, char const* code, size_t codeLen) {
    uint64_t h = 0;
    if (path) for (; *path; ++path) h = h * 31u + static_cast<unsigned char>(*path);
    h = h * 31u + ':';
    for (size_t i = 0; i < codeLen; ++i) h = h * 31u + static_cast<unsigned char>(code[i]);
    return h;
}
std::string makeKey(ShaderHandleImpl* h) {
    std::ostringstream os;
    os << (h->sourcePath_.empty() ? "(memory)" : h->sourcePath_)
       << ":" << hashSource(h->sourcePath_.c_str(), h->sourceCode_.data(), h->sourceCode_.size());
    return os.str();
}
static constexpr char const* kMagic = "TESC\x01\0\0\0";
static constexpr uint32_t kVersion = 1;
}  // namespace

void ShaderCacheImpl::StoreFromHandle(ShaderHandleImpl* handle) {
    if (!handle || handle->variantBytecode_.empty()) return;
    std::string key = makeKey(handle);
    auto& map = store_[key];
    for (auto const& p : handle->variantBytecode_) {
        map[p.first] = p.second;
    }
}

bool ShaderCacheImpl::TryLoadToHandle(ShaderHandleImpl* handle) {
    if (!handle) return false;
    std::string key = makeKey(handle);
    auto it = store_.find(key);
    if (it == store_.end()) return false;
    for (auto const& p : it->second) {
        handle->variantBytecode_[p.first] = p.second;
    }
    return !it->second.empty();
}

void ShaderCacheImpl::Invalidate(IShaderHandle* handle) {
    if (!handle) return;
    auto* h = static_cast<ShaderHandleImpl*>(handle);
    h->variantBytecode_.clear();
    h->bytecode_.clear();
    h->bytecodeBlob_.clear();
    h->crossCompiledSource_.clear();
    std::string key = makeKey(h);
    store_.erase(key);
}

bool ShaderCacheImpl::LoadCache(char const* path) {
    if (!path) return false;
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    std::string normPath = te::core::PathNormalize(path);
    auto opt = te::core::FileRead(normPath);
    if (!opt || opt->size() < 12) {
        if (opt && opt->empty()) return true;
        if (opt && opt->size() > 0 && opt->size() < 12) return false;
        return true;
    }
    std::vector<std::uint8_t> const& data = *opt;
#else
    std::ifstream f(path, std::ios::binary);
    if (!f) return true;
    std::vector<std::uint8_t> data((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    f.close();
    if (data.size() < 12) return true;
#endif
    if (std::memcmp(data.data(), kMagic, 8) != 0) return false;
    uint32_t ver;
    std::memcpy(&ver, data.data() + 8, 4);
    if (ver != kVersion) return false;
    size_t pos = 12;
    uint32_t numEntries;
    if (pos + 4 > data.size()) return false;
    std::memcpy(&numEntries, data.data() + pos, 4);
    pos += 4;
    for (uint32_t e = 0; e < numEntries && pos < data.size(); ++e) {
        if (pos + 4 > data.size()) break;
        uint32_t keyLen;
        std::memcpy(&keyLen, data.data() + pos, 4);
        pos += 4;
        if (pos + keyLen > data.size()) break;
        std::string key(reinterpret_cast<char const*>(data.data() + pos), keyLen);
        pos += keyLen;
        uint32_t numVars;
        if (pos + 4 > data.size()) break;
        std::memcpy(&numVars, data.data() + pos, 4);
        pos += 4;
        auto& varMap = store_[key];
        for (uint32_t v = 0; v < numVars && pos < data.size(); ++v) {
            if (pos + 8 > data.size()) break;
            uint64_t hash;
            std::memcpy(&hash, data.data() + pos, 8);
            pos += 8;
            VariantBytecode vb;
            if (pos + 4 > data.size()) break;
            uint32_t spirvCount;
            std::memcpy(&spirvCount, data.data() + pos, 4);
            pos += 4;
            if (spirvCount > 0 && pos + spirvCount * 4 <= data.size()) {
                vb.spirv.resize(spirvCount);
                std::memcpy(vb.spirv.data(), data.data() + pos, spirvCount * 4);
                pos += spirvCount * 4;
            }
            if (pos + 4 > data.size()) break;
            uint32_t dxilSize;
            std::memcpy(&dxilSize, data.data() + pos, 4);
            pos += 4;
            if (dxilSize > 0 && pos + dxilSize <= data.size()) {
                vb.dxil.resize(dxilSize);
                std::memcpy(vb.dxil.data(), data.data() + pos, dxilSize);
                pos += dxilSize;
            }
            if (pos + 4 > data.size()) break;
            uint32_t crossSize;
            std::memcpy(&crossSize, data.data() + pos, 4);
            pos += 4;
            if (crossSize > 0 && pos + crossSize <= data.size()) {
                vb.crossCompiled.assign(reinterpret_cast<char const*>(data.data() + pos), crossSize);
                pos += crossSize;
            }
            varMap[hash] = std::move(vb);
        }
    }
    return true;
}

bool ShaderCacheImpl::SaveCache(char const* path) {
    if (!path) return false;
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    std::string normPath = te::core::PathNormalize(path);
#else
    std::string normPath = path;
#endif
    std::vector<std::uint8_t> data;
    data.insert(data.end(), kMagic, kMagic + 8);
    uint32_t ver = kVersion;
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&ver), reinterpret_cast<uint8_t*>(&ver) + 4);
    uint32_t numEntries = static_cast<uint32_t>(store_.size());
    data.insert(data.end(), reinterpret_cast<uint8_t*>(&numEntries), reinterpret_cast<uint8_t*>(&numEntries) + 4);
    for (auto const& entry : store_) {
        uint32_t keyLen = static_cast<uint32_t>(entry.first.size());
        data.insert(data.end(), reinterpret_cast<uint8_t*>(&keyLen), reinterpret_cast<uint8_t*>(&keyLen) + 4);
        data.insert(data.end(), entry.first.begin(), entry.first.end());
        uint32_t numVars = static_cast<uint32_t>(entry.second.size());
        data.insert(data.end(), reinterpret_cast<uint8_t*>(&numVars), reinterpret_cast<uint8_t*>(&numVars) + 4);
        for (auto const& v : entry.second) {
            data.insert(data.end(), reinterpret_cast<uint8_t const*>(&v.first), reinterpret_cast<uint8_t const*>(&v.first) + 8);
            uint32_t spirvCount = static_cast<uint32_t>(v.second.spirv.size());
            data.insert(data.end(), reinterpret_cast<uint8_t*>(&spirvCount), reinterpret_cast<uint8_t*>(&spirvCount) + 4);
            if (!v.second.spirv.empty()) {
                data.insert(data.end(), reinterpret_cast<uint8_t const*>(v.second.spirv.data()),
                            reinterpret_cast<uint8_t const*>(v.second.spirv.data()) + v.second.spirv.size() * 4);
            }
            uint32_t dxilSize = static_cast<uint32_t>(v.second.dxil.size());
            data.insert(data.end(), reinterpret_cast<uint8_t*>(&dxilSize), reinterpret_cast<uint8_t*>(&dxilSize) + 4);
            if (!v.second.dxil.empty()) {
                data.insert(data.end(), v.second.dxil.begin(), v.second.dxil.end());
            }
            uint32_t crossSize = static_cast<uint32_t>(v.second.crossCompiled.size());
            data.insert(data.end(), reinterpret_cast<uint8_t*>(&crossSize), reinterpret_cast<uint8_t*>(&crossSize) + 4);
            if (!v.second.crossCompiled.empty()) {
                data.insert(data.end(), v.second.crossCompiled.begin(), v.second.crossCompiled.end());
            }
        }
    }
#if defined(TE_SHADER_USE_CORE) && TE_SHADER_USE_CORE
    if (!te::core::FileWrite(normPath, data)) {
        te::core::Log(te::core::LogLevel::Error, "ShaderCacheImpl::SaveCache: FileWrite failed");
        return false;
    }
#else
    std::ofstream f(normPath, std::ios::binary);
    if (!f) return false;
    f.write(reinterpret_cast<char const*>(data.data()), data.size());
    if (!f) return false;
#endif
    return true;
}

}  // namespace te::shader
